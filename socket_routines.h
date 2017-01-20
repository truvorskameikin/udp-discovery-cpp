#ifndef __SOCKET_ROUTINES_H_
#define __SOCKET_ROUTINES_H_

#if defined(_WIN32)
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
namespace socketroutines {
  typedef SOCKET SocketType;
  typedef int AddressLenType;
  const SocketType kInvalidSocket = INVALID_SOCKET;
}
#endif

#if defined(__APPLE__) || defined(__ANDROID__) || defined(__gnu_linux__)
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
namespace socketroutines {
  typedef int SocketType;
  typedef socklen_t AddressLenType;
  const SocketType kInvalidSocket = -1;
}
#endif

namespace socketroutines {
  static
  bool InitSockets() {
#if defined(_WIN32)
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0)
        return true;
    return false;
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    return true;
  }

  inline
  void CloseSocket(socketroutines::SocketType s)
  {
#if defined(_WIN32)
    closesocket(s);
#else
    close(s);
#endif
  }

  inline
  void MakeSocketNoblock(socketroutines::SocketType s) {
#if defined(_WIN32)
    unsigned long is_blocking = true;
    ioctlsocket(s, FIONBIO, &is_blocking);
#else
    int flags = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
  }

  inline
  bool SocketDroppedConnection() {
#if defined(_WIN32)
    int e = WSAGetLastError();
    return (e != WSAEWOULDBLOCK);
#else
    int e = errno;
    return (e != EAGAIN && e != EWOULDBLOCK);
#endif
  }

  class SocketGuard {
   public:
    SocketGuard(SocketType socket) : socket_(socket) {
    }

    ~SocketGuard() {
      socketroutines::CloseSocket(socket_);
    }

    SocketType socket() {
      return socket_;
    }

   private:
    SocketType socket_;
  };
}

#endif
