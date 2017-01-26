#include <vector>
#include <iostream>
#include "protocol.hpp"
#include "server.hpp"

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

// threads
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <stdlib.h>
#include <pthread.h>
#endif

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
    class ServerWorkingEnv : public ServerWorkingEnvInterface {
     public:
      ServerWorkingEnv() : application_id_(0), sock_(kInvalidSocket), exit_(false) {
#if defined(_WIN32)
        InitializeCriticalSection(&critical_section_);
#else
        pthread_mutex_init(&mutex_, 0);
#endif
      }

      ~ServerWorkingEnv() {
        if (sock_ != kInvalidSocket) {
#if defined(_WIN32)
          closesocket(sock_);
#else
          close(sock_);
#endif
        }

#if defined(_WIN32)
        DeleteCriticalSection(&critical_section_);
#else
        pthread_mutex_destroy(&mutex_);
#endif
      }

      bool StartServer(int port, uint64_t application_id) {
#if defined(_WIN32)
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == kInvalidSocket) {
          std::cerr << "udpdiscovery::Server can't create socket" << std::endl;
          return false;
        }

        int value = 1;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char *) &value, sizeof(value));

        const int receive_timeout = 500;
#if defined(_WIN32)
        {
          int timeout = receive_timeout;
          setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));
        }
#else
        {
          struct timeval timeout;
          timeout.tv_sec = receive_timeout / 1000;
          timeout.tv_usec = 1000 * (receive_timeout % 1000);
          setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));
        }
#endif

        sockaddr_in addr;
        memset((char *) &addr, 0, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock_, (struct sockaddr *) &addr, sizeof(sockaddr_in)) < 0) {
          if (sock_ != kInvalidSocket) {
#if defined(_WIN32)
            closesocket(sock_);
#else
            close(sock_);
#endif
          }

          sock_ = kInvalidSocket;

          std::cerr << "udpdiscovery::Server can't bind socket" << std::endl;
          return false;
        }

        application_id_ = application_id;
        buffer_.resize(kMaxPacketSize);

        return true;
      }

      void Update() {
        sockaddr_in from_addr;
        AddressLenType addr_length = sizeof(sockaddr_in);

        int length = (int) recvfrom(sock_, &buffer_[0], buffer_.size(), 0, (struct sockaddr *) &from_addr, &addr_length);
        if (length <= 0) {
          lock();
          deleteIdle(NowTime());
          unlock();

          return;
        }

        IpPort from;
        from.set_port(ntohs(from_addr.sin_port));
        from.set_ip(ntohl(from_addr.sin_addr.s_addr));

        if (length >= sizeof(PacketHeader)) {
          PacketHeader header;
          if (ParsePacketHeader(buffer_.data(), sizeof(PacketHeader), header)) {
            if (application_id_ == header.application_id) {
              std::string user_data(
                buffer_.begin() + sizeof(PacketHeader), buffer_.begin() + sizeof(PacketHeader) + header.user_data_size);

              lock();

              std::list<DiscoveredClient>::iterator find_it = discovered_clients_.end();
              for (std::list<DiscoveredClient>::iterator it = discovered_clients_.begin(); it != discovered_clients_.end(); ++it) {
                if (Same((*it).ip_port(), from)) {
                  find_it = it;
                  break;
                }
              }

              long cur_time = NowTime();

              if (find_it == discovered_clients_.end()) {
                discovered_clients_.push_back(DiscoveredClient());
                discovered_clients_.back().set_ip_port(from);
                discovered_clients_.back().SetUserData(user_data, header.packet_index);
                discovered_clients_.back().set_last_updated(cur_time);
              } else {
                bool new_user_data = false;
                if (header.packet_index_reset) {
                  new_user_data = true;
                } else {
                  if ((*find_it).last_received_packet() < header.packet_index)
                    new_user_data = true;
                }

                if (new_user_data)
                  (*find_it).SetUserData(user_data, header.packet_index);
                (*find_it).set_last_updated(cur_time);
              }

              deleteIdle(cur_time);

              unlock();
            }
          }
        }
      }

      std::list<DiscoveredClient> ListClients() {
        lock();
        std::list<DiscoveredClient> result = discovered_clients_;
        unlock();

        return result;
      }

      void Exit() {
        lock();
        exit_ = true;
        unlock();
      }

      bool CanExit() {
        lock();
        bool result = exit_;
        unlock();

        return result;
      }

     private:
      void deleteIdle(long cur_time) {
        long idle_timeout = 5000;

        std::vector<std::list<DiscoveredClient>::iterator> to_delete;
        for (std::list<DiscoveredClient>::iterator it = discovered_clients_.begin(); it != discovered_clients_.end(); ++it) {
          if (cur_time - (*it).last_updated() > idle_timeout)
            to_delete.push_back(it);
        }

        for (size_t i = 0; i < to_delete.size(); ++i)
          discovered_clients_.erase(to_delete[i]);
      }

     private:
      uint64_t application_id_;
      std::string buffer_;
      SocketType sock_;

      void lock() {
#if defined(_WIN32)
        EnterCriticalSection(&critical_section_);
#else
        pthread_mutex_lock(&mutex_);
#endif
      }

      void unlock() {
#if defined(_WIN32)
        LeaveCriticalSection(&critical_section_);
#else
        pthread_mutex_unlock(&mutex_);
#endif
      }

#if defined(_WIN32)
      CRITICAL_SECTION critical_section_;
#else
      pthread_mutex_t mutex_;
#endif
      bool exit_;
      std::list<DiscoveredClient> discovered_clients_;
    };

    void ServerWorkingFunc(ServerWorkingEnv* working_env) {
      while (!working_env->CanExit()) {
        working_env->Update();
      }

      delete working_env;
    }

#if defined(_WIN32)
    DWORD WINAPI PlatformServerWorkingFunc(void* working_env_typeless) {
      ServerWorkingEnv* working_env = (ServerWorkingEnv*) working_env_typeless;
      ServerWorkingFunc(working_env);

      return 0;
    }
#else
    void* PlatformServerWorkingFunc(void* working_env_typeless) {
      ServerWorkingEnv* working_env = (ServerWorkingEnv*) working_env_typeless;
      ServerWorkingFunc(working_env);

      return 0;
    }
#endif
  };

  Server::Server() : started_(false) {
  }

  Server::~Server() {
    Stop();
  }

  bool Server::Start(int port, uint64_t application_id) {
    Stop();

    impl::ServerWorkingEnv* working_env = new impl::ServerWorkingEnv();
    if (!working_env->StartServer(port, application_id)) {
      delete working_env;
      return false;
    }

    working_env_ = working_env;

#if defined(_WIN32)
    HANDLE thread = CreateThread(NULL, 0, impl::PlatformServerWorkingFunc, working_env_, 0, NULL);
    CloseHandle(thread);
#else
    pthread_t thread;
    pthread_create(&thread, 0, impl::PlatformServerWorkingFunc, working_env_);
    pthread_detach(thread);
#endif

    started_ = true;

    return true;
  }

  std::list<DiscoveredClient> Server::ListClients() const {
    std::list<DiscoveredClient> empty_discovered_clients;
    if (!started_)
      return empty_discovered_clients;
    return working_env_->ListClients();
  }

  void Server::Stop() {
    if (!started_)
      return;

    working_env_->Exit();
    working_env_ = 0;

    started_ = false;
  }

  bool Same(const IpPort& lhv, const IpPort& rhv) {
    return lhv == rhv;
  }

  bool Same(const std::list<DiscoveredClient>& lhv, const std::list<DiscoveredClient>& rhv) {
    for (std::list<DiscoveredClient>::const_iterator lhv_it = lhv.begin(); lhv_it != lhv.end(); ++lhv_it) {
      std::list<DiscoveredClient>::const_iterator in_rhv = rhv.end();
      for (std::list<DiscoveredClient>::const_iterator rhv_it = rhv.begin(); rhv_it != rhv.end(); ++rhv_it) {
        if (Same((*lhv_it).ip_port(), (*rhv_it).ip_port())) {
          in_rhv = rhv_it;
          break;
        }
      }

      if (in_rhv == rhv.end())
        return false;
    }

    for (std::list<DiscoveredClient>::const_iterator rhv_it = rhv.begin(); rhv_it != rhv.end(); ++rhv_it) {
      std::list<DiscoveredClient>::const_iterator in_lhv = lhv.end();
      for (std::list<DiscoveredClient>::const_iterator lhv_it = lhv.begin(); lhv_it != lhv.end(); ++lhv_it) {
        if (Same((*rhv_it).ip_port(), (*lhv_it).ip_port())) {
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
