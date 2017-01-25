#include <iostream>
#include "protocol.hpp"
#include "client.hpp"

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

namespace udpdiscovery {
  namespace impl {
    class ClientWorkingEnv : public ClientWorkingEnvInterface {
     public:
      ClientWorkingEnv()
          : port_(0),
            sock_(kInvalidSocket),
            packet_index_(0),
            max_packet_index_(1073741824),
            exit_(false) {
#if defined(_WIN32)
        InitializeCriticalSection(&critical_section_);
#else
        pthread_mutex_init(&mutex_, 0);
#endif
      }

      ~ClientWorkingEnv() {
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

      bool StartClient(int port) {
#if defined(_WIN32)
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == kInvalidSocket) {
          std::cerr << "udpdiscovery::Client can't create socket" << std::endl;
          return false;
        }

        int value = 1;
        setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, (const char *) &value, sizeof(value));

        port_ = port;
        packet_index_ = 0;

        return true;
      }

      void Send() {
        lock();
        std::string user_data = user_data_;
        unlock();

        PacketHeader header;

        header.packet_type = kPacketIAmHere;

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
          addr.sin_port = htons(port_);
          addr.sin_addr.s_addr = htonl(0xffffffff);

          sendto(sock_, &packet_data[0], packet_data.size(), 0, (struct sockaddr *) &addr, sizeof(sockaddr_in));
        }
      }

      void SetUserData(const std::string& user_data) {
        lock();
        user_data_ = user_data;
        unlock();
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
      int port_;
      SocketType sock_;
      uint64_t packet_index_;
      uint64_t max_packet_index_;

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
      std::string user_data_;
    };

    void ClientWorkingFunc(ClientWorkingEnv* working_env) {
      int send_timeout = 1000;
      while (!working_env->CanExit()) {
        working_env->Send();

#if defined(_WIN32)
        Sleep(send_timeout);
#else
        usleep(send_timeout * 1000);
#endif
      }

      delete working_env;
    }

#if defined(_WIN32)
    DWORD WINAPI PlatformClientWorkingFunc(void* working_env_typeless) {
      ClientWorkingEnv* working_env = (ClientWorkingEnv*) working_env_typeless;
      ClientWorkingFunc(working_env);

      return 0;
    }
#else
    void* PlatformClientWorkingFunc(void* working_env_typeless) {
      ClientWorkingEnv* working_env = (ClientWorkingEnv*) working_env_typeless;
      ClientWorkingFunc(working_env);

      return 0;
    }
#endif
  };

  Client::Client() : started_(false) {
  }

  Client::~Client() {
    Stop();
  }

  bool Client::Start(int port, const std::string& user_data) {
    Stop();

    impl::ClientWorkingEnv* working_env = new impl::ClientWorkingEnv();
    if (!working_env->StartClient(port)) {
      delete working_env;
      return false;
    }

    working_env_ = working_env;
    working_env_->SetUserData(user_data);

#if defined(_WIN32)
    HANDLE thread = CreateThread(NULL, 0, impl::PlatformClientWorkingFunc, working_env_, 0, NULL);
    CloseHandle(thread);
#else
    pthread_t thread;
    pthread_create(&thread, 0, impl::PlatformClientWorkingFunc, working_env_);
    pthread_detach(thread);
#endif

    started_ = true;

    return true;
  }

  void Client::SetUserData(const std::string& user_data) {
    if (!started_)
      return;

    working_env_->SetUserData(user_data);
  }

  void Client::Stop() {
    if (!started_)
      return;

    working_env_->Exit();
    working_env_ = 0;

    started_ = false;
  }
};
