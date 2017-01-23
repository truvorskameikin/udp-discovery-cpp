#include "socket_routines.h"
#include "protocol.hpp"
#include "Client.hpp"

namespace udpdiscovery {
  namespace impl {
    class ClientSocket : public ClientSocketInterface {
     public:
      ClientSocket() : sock_(socketroutines::kInvalidSocket) {
      }

      ~ClientSocket() {
        socketroutines::CloseSocket(sock_);
      }

      bool Create(int port) {
        sock_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock_ == socketroutines::kInvalidSocket)
          return false;

        int value = 1;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, (const char *) &value, sizeof(value));

        value = 1;
        setsockopt(sock_, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value));

        port_ = port;

        return true;
      }

      int Send(const char* buffer, int buffer_size) {
        sockaddr_in addr;
        memset((char *) &addr, 0, sizeof(sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = htonl(-1);

        return sendto(sock_, buffer, buffer_size, 0, (struct sockaddr *) &addr, sizeof(sockaddr_in));
      }

     private:
      int port_;
      socketroutines::SocketType sock_;
    };
  };

  Client::Client() : started_(false){
    sock_ = new impl::ClientSocket;
  }

  Client::~Client() {
    delete sock_;
  }

  bool Client::Start(int port, const std::string& user_data) {
    if (!sock_->Create(port))
      return false;

    started_ = true;
    user_data_ = user_data;
    max_packet_index_ = 1073741824;

    return true;
  }

  void Client::Send() {
    if (!started_)
      return;

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
    if (MakePacket(header, user_data_, packet_data)) {
      sock_->Send(packet_data.c_str(), packet_data.size());
    }
  }
};
