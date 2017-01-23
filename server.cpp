#include "socket_routines.h"
#include "protocol.hpp"
#include "server.hpp"

#include <iostream>

namespace udpdiscovery {
  namespace impl {
    class ServerSocket : public ServerSocketInterface {
     public:
      ServerSocket() : sock_(socketroutines::kInvalidSocket) {
      }

      bool Create(int port) {
        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == socketroutines::kInvalidSocket)
          return false;

        const int value = 1;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char *) &value, sizeof(value));

        sockaddr_in addr;
        memset((char *) &addr, 0, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(sock_, (struct sockaddr *) &addr, sizeof(sockaddr_in)) < 0)
          return false;

        return true;
      }

      int Recv(char* buffer, int buffer_size, IpPort& from_out) {
        sockaddr_in from_addr;
        socketroutines::AddressLenType addr_length = sizeof(sockaddr_in);

        int length = (int) recvfrom(sock_, buffer, buffer_size, 0, (struct sockaddr *) &from_addr, &addr_length);
        if (length <= 0)
          return length;

        from_out.set_port(ntohs(from_addr.sin_port));
        from_out.set_ip(ntohl(from_addr.sin_addr.s_addr));

        return length;
      }

     private:
      socketroutines::SocketType sock_;
    };
  };

  Server::Server() : started_(false) {
    sock_ = new impl::ServerSocket;
  }

  Server::~Server() {
    delete sock_;
  }

  bool Server::Start(int port) {
    if (!sock_->Create(port))
      return false;

    started_ = true;
    buffer_.resize(kMaxPacketSize);

    return true;
  }

  void Server::Update() {
    if (!started_)
      return;

    IpPort from;
    int length = sock_->Recv(&buffer_[0], kMaxPacketSize, from);

    if (length >= sizeof(PacketHeader)) {
      PacketHeader header;
      if (ParsePacketHeader(buffer_.data(), sizeof(PacketHeader), header)) {
        std::string user_data(
          buffer_.begin() + sizeof(PacketHeader), buffer_.begin() + sizeof(PacketHeader) + header.user_data_size);
        std::cout << "From " << from.ip() << ":" << from.port() << ": " << user_data << std::endl;
      }
    }
  }
};
