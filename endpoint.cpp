#include <vector>
#include <iostream>
#include "protocol.hpp"
#include "endpoint.hpp"

// sockets
#if defined(_WIN32)
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SocketType;
typedef int AddressLenType;
const SocketType kInvalidSocket = INVALID_SOCKET;
#else
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
typedef int SocketType;
typedef socklen_t AddressLenType;
const SocketType kInvalidSocket = -1;
#endif

void InitSockets() {
#if defined(_WIN32)
  WSADATA wsa_data;
  WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
}

void CloseSocket(SocketType sock) {
#if defined(_WIN32)
  closesocket(sock);
#else
  close(sock);
#endif
}

void SetSocketTimeout(SocketType sock, int param, int timeout_ms) {
#if defined(_WIN32)
    setsockopt(sock, SOL_SOCKET, param, (const char *) &timeout_ms, sizeof(timeout_ms));
#else
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = 1000 * (timeout_ms % 1000);
    setsockopt(sock, SOL_SOCKET, param, (const char *) &timeout, sizeof(timeout));
#endif
}

// threads
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdlib.h>
#include <pthread.h>
#endif

void SleepFor(int time_ms) {
#if defined(_WIN32)
  Sleep(time_ms);
#else
  usleep(time_ms * 1000);
#endif
}

class MinimalisticMutex {
public:
  MinimalisticMutex() {
#if defined(_WIN32)
    InitializeCriticalSection(&critical_section_);
#else
    pthread_mutex_init(&mutex_, 0);
#endif
  }

  ~MinimalisticMutex() {
#if defined(_WIN32)
    DeleteCriticalSection(&critical_section_);
#else
    pthread_mutex_destroy(&mutex_);
#endif
  }

  void Lock() {
#if defined(_WIN32)
    EnterCriticalSection(&critical_section_);
#else
    pthread_mutex_lock(&mutex_);
#endif
  }

  void Unlock() {
#if defined(_WIN32)
    LeaveCriticalSection(&critical_section_);
#else
    pthread_mutex_unlock(&mutex_);
#endif
  }

private:
#if defined(_WIN32)
  CRITICAL_SECTION critical_section_;
#else
  pthread_mutex_t mutex_;
#endif
};

// time
#if defined(__APPLE__)
#include <stdint.h>
#include <mach/mach_time.h>
#endif
#if !defined(_WIN32)
#include <sys/time.h>
#endif
#include <time.h>

static
long NowTime() {
#if defined(_WIN32)
  LARGE_INTEGER freq;
  if (!QueryPerformanceFrequency(&freq))
    return 0;
  LARGE_INTEGER cur;
  QueryPerformanceCounter(&cur);
  return (long) (cur.QuadPart * 1000 / freq.QuadPart);
#elif defined(__APPLE__)
  mach_timebase_info_data_t time_info;
  mach_timebase_info(&time_info);

  uint64_t cur = mach_absolute_time();
  return (long) ((cur / (time_info.denom * 1000000)) * time_info.numer);
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long) (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif

  return 0;
}

namespace udpdiscovery {
  namespace impl {
    class EndpointEnv : public EndpointEnvInterface {
     public:
      EndpointEnv()
          : sock_(kInvalidSocket),
            packet_index_(0),
            max_packet_index_(1073741824),
            exit_(false) {
      }

      ~EndpointEnv() {
        CloseSocket(sock_);
      }

      bool Start(const EndpointParameters& parameters, const std::string& user_data) {
        parameters_ = parameters;
        user_data_ = user_data;

        if (!parameters_.can_discover() && !parameters_.can_be_discovered()) {
          std::cerr << "udpdiscovery::Endpoint can't discover and can't be discovered" << std::endl;
          return false;
        }

        InitSockets();

        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == kInvalidSocket) {
          std::cerr << "udpdiscovery::Endpoint can't create socket" << std::endl;
          return false;
        }

        {
          int value = 1;
          setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, (const char *) &value, sizeof(value));
        }

        if (parameters_.can_discover()) {
          {
            int value = 1;
            setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char *) &value, sizeof(value));
          }

          SetSocketTimeout(sock_, SO_RCVTIMEO, parameters_.receive_timeout_ms());

          sockaddr_in addr;
          memset((char *) &addr, 0, sizeof(sockaddr_in));
          addr.sin_family = AF_INET;
          addr.sin_port = htons(parameters_.port());
          addr.sin_addr.s_addr = htonl(INADDR_ANY);

          if (bind(sock_, (struct sockaddr *) &addr, sizeof(sockaddr_in)) < 0) {
            CloseSocket(sock_);
            sock_ = kInvalidSocket;

            std::cerr << "udpdiscovery::Endpoint can't bind socket" << std::endl;
            return false;
          }

          buffer_.resize(kMaxPacketSize);
        }

        return true;
      }

      void SetUserData(const std::string& user_data) {
        lock_.Lock();
        user_data_ = user_data;
        lock_.Unlock();
      }

      std::list<DiscoveredEndpoint> ListDiscovered() {
        std::list<DiscoveredEndpoint> result;

        lock_.Lock();
        result = discovered_endpoints_;
        lock_.Unlock();

        return result;
      }

      void Exit() {
        lock_.Lock();
        exit_ = true;
        lock_.Unlock();
      }

      void DoWork() {
        long last_send_time = 0;
        while (true) {
          lock_.Lock();
          bool exit = exit_;
          lock_.Unlock();

          if (exit)
            break;

          if (parameters_.can_be_discovered()) {
            long cur_time = NowTime();
            if (last_send_time == 0) {
              send(kPacketIAmHere);
              last_send_time = cur_time;
            } else {
              if (cur_time - last_send_time > parameters_.send_timeout_ms()) {
                send(kPacketIAmHere);
                last_send_time = cur_time;
              }
            }
          }

          if (parameters_.can_discover()) {
            receive();
            deleteIdle(NowTime());
          } else {
            SleepFor(parameters_.receive_timeout_ms());
          }
        }

        if (parameters_.can_be_discovered())
          send(kPacketIAmOutOfHere);
      }

     private:
      void receive() {
        sockaddr_in from_addr;
        AddressLenType addr_length = sizeof(sockaddr_in);

        int length = (int) recvfrom(sock_, &buffer_[0], buffer_.size(), 0, (struct sockaddr *) &from_addr, &addr_length);
        if (length <= 0)
          return;

        IpPort from;
        from.set_port(ntohs(from_addr.sin_port));
        from.set_ip(ntohl(from_addr.sin_addr.s_addr));

        if (length >= sizeof(PacketHeader)) {
          PacketHeader header;
          if (ParsePacketHeader(buffer_.data(), sizeof(PacketHeader), header)) {
            if (parameters_.application_id() == header.application_id) {
              std::string user_data(
                buffer_.begin() + sizeof(PacketHeader), buffer_.begin() + sizeof(PacketHeader) + header.user_data_size);

              lock_.Lock();

              std::list<DiscoveredEndpoint>::iterator find_it = discovered_endpoints_.end();
              for (std::list<DiscoveredEndpoint>::iterator it = discovered_endpoints_.begin(); it != discovered_endpoints_.end(); ++it) {
                if (Same(parameters_.same_endpoint_mode(), (*it).ip_port(), from)) {
                  find_it = it;
                  break;
                }
              }

              long cur_time = NowTime();

              if (header.packet_type == kPacketIAmHere) {
                if (find_it == discovered_endpoints_.end()) {
                  discovered_endpoints_.push_back(DiscoveredEndpoint());
                  discovered_endpoints_.back().set_ip_port(from);
                  discovered_endpoints_.back().SetUserData(user_data, header.packet_index);
                  discovered_endpoints_.back().set_last_updated(cur_time);
                } else {
                  bool update_user_data = false;
                  if (header.packet_index_reset) {
                    update_user_data = true;
                  } else {
                    if ((*find_it).last_received_packet() < header.packet_index)
                      update_user_data = true;
                  }

                  if (update_user_data)
                    (*find_it).SetUserData(user_data, header.packet_index);
                  (*find_it).set_last_updated(cur_time);
                }
              } else if (header.packet_type == kPacketIAmOutOfHere) {
                if (find_it != discovered_endpoints_.end()) {
                  discovered_endpoints_.erase(find_it);
                }
              }

              lock_.Unlock();
            }
          }
        }
      }

      void deleteIdle(long cur_time) {
        lock_.Lock();

        std::vector<std::list<DiscoveredEndpoint>::iterator> to_delete;
        for (std::list<DiscoveredEndpoint>::iterator it = discovered_endpoints_.begin(); it != discovered_endpoints_.end(); ++it) {
          if (cur_time - (*it).last_updated() > parameters_.discovered_endpoint_ttl_ms())
            to_delete.push_back(it);
        }

        for (size_t i = 0; i < to_delete.size(); ++i)
          discovered_endpoints_.erase(to_delete[i]);

        lock_.Unlock();
      }

      void send(PacketType packet_type) {
        lock_.Lock();
        std::string user_data = user_data_;
        lock_.Unlock();

        PacketHeader header;

        header.packet_type = packet_type;

        header.application_id = parameters_.application_id();
        header.packet_index = packet_index_;
        if (header.packet_index >= max_packet_index_) {
          packet_index_ = 0;

          header.packet_index = 0;
          header.packet_index_reset = 1;
        }

        ++packet_index_;

        std::string packet_data;
        if (MakePacket(header, user_data, packet_data)) {
          sockaddr_in addr;
          memset((char *) &addr, 0, sizeof(sockaddr_in));
          addr.sin_family = AF_INET;
          addr.sin_port = htons(parameters_.port());
          addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

          sendto(sock_, &packet_data[0], packet_data.size(), 0, (struct sockaddr *) &addr, sizeof(sockaddr_in));
        }
      }

     private:
      EndpointParameters parameters_;
      std::vector<char> buffer_;
      SocketType sock_;
      uint64_t packet_index_;
      uint64_t max_packet_index_;

      MinimalisticMutex lock_;
      bool exit_;
      std::string user_data_;
      std::list<DiscoveredEndpoint> discovered_endpoints_;
    };

    class MinimalisticThread : public MinimalisticThreadInterface {
     public:
#if defined(_WIN32)
      MinimalisticThread(LPTHREAD_START_ROUTINE f, void* env) : detached_(false) {
        thread_ = CreateThread(NULL, 0, f, env, 0, NULL);
      }
#else
      MinimalisticThread(void* (*f)(void*), void* env) : detached_(false) {
        pthread_create(&thread_, 0, f, env);
      }
#endif

      ~MinimalisticThread() {
        Detach();
      }

      void Detach() {
        if (detached_)
          return;

#if defined(_WIN32)
        CloseHandle(thread_);
#else
        pthread_detach(thread_);
#endif
        detached_ = true;
      }

      void Join() {
        if (detached_)
          return;

#if defined(_WIN32)
        WaitForSingleObject(thread_, INFINITE);
        CloseHandle(thread_);
#else
        pthread_join(thread_, 0);
#endif
        detached_ = true;
      }

     private:
      bool detached_;
#if defined(_WIN32)
      HANDLE thread_;
#else
      pthread_t thread_;
#endif
    };

#if defined(_WIN32)
    DWORD WINAPI EndpointWork(void* env_typeless) {
      EndpointEnv* env = (EndpointEnv*) env_typeless;
      env->DoWork();

	  return 0;
    }
#else
    void* EndpointWork(void* env_typeless) {
      EndpointEnv* env = (EndpointEnv*) env_typeless;
      env->DoWork();

      return 0;
    }
#endif
  };

  Endpoint::Endpoint() : env_(0), thread_(0) {
  }

  Endpoint::~Endpoint() {
    Stop(false);
  }

  bool Endpoint::Start(const EndpointParameters& parameters, const std::string& user_data) {
    Stop(false);

    impl::EndpointEnv* env = new impl::EndpointEnv();
    if (!env->Start(parameters, user_data)) {
      delete env;
      env = 0;

      return false;
    }

    impl::MinimalisticThread* thread = new impl::MinimalisticThread(impl::EndpointWork, env);

    env_ = env;
    thread_ = thread;

    return true;
  }

  void Endpoint::SetUserData(const std::string& user_data) {
    if (env_)
      env_->SetUserData(user_data);
  }

  std::list<DiscoveredEndpoint> Endpoint::ListDiscovered() const {
    std::list<DiscoveredEndpoint> result;
    if (env_)
      result = env_->ListDiscovered();
    return result;
  }

  void Endpoint::Stop(bool wait_for_thread) {
    if (!env_)
      return;

    env_->Exit();
    env_ = 0;

    if (wait_for_thread)
      thread_->Join();
    else
      thread_->Detach();

    delete thread_;
    thread_ = 0;
  }

  bool Same(EndpointParameters::SameEndpointMode mode, const IpPort& lhv, const IpPort& rhv) {
    switch (mode) {
    case EndpointParameters::kSameEndpointIp:
      return lhv.ip() == rhv.ip();

    case EndpointParameters::kSameEndpointIpAndPort:
      return (lhv.ip() == rhv.ip()) && (lhv.port() == rhv.port());
    }

    return false;
  }

  bool Same(EndpointParameters::SameEndpointMode mode, const std::list<DiscoveredEndpoint>& lhv, const std::list<DiscoveredEndpoint>& rhv) {
    for (std::list<DiscoveredEndpoint>::const_iterator lhv_it = lhv.begin(); lhv_it != lhv.end(); ++lhv_it) {
      std::list<DiscoveredEndpoint>::const_iterator in_rhv = rhv.end();
      for (std::list<DiscoveredEndpoint>::const_iterator rhv_it = rhv.begin(); rhv_it != rhv.end(); ++rhv_it) {
        if (Same(mode, (*lhv_it).ip_port(), (*rhv_it).ip_port())) {
          in_rhv = rhv_it;
          break;
        }
      }

      if (in_rhv == rhv.end())
        return false;
    }

    for (std::list<DiscoveredEndpoint>::const_iterator rhv_it = rhv.begin(); rhv_it != rhv.end(); ++rhv_it) {
      std::list<DiscoveredEndpoint>::const_iterator in_lhv = lhv.end();
      for (std::list<DiscoveredEndpoint>::const_iterator lhv_it = lhv.begin(); lhv_it != lhv.end(); ++lhv_it) {
        if (Same(mode, (*rhv_it).ip_port(), (*lhv_it).ip_port())) {
          in_lhv = lhv_it;
          break;
        }
      }

      if (in_lhv == lhv.end())
        return false;
    }

    return true;
  }
};
