#ifndef __UDP_DISCOVERY_SERVER_H_
#define __UDP_DISCOVERY_SERVER_H_

#include "ip_port.hpp"

namespace udpdiscovery {
  namespace impl {
    class ServerSocketInterface {
     public:
      virtual ~ServerSocketInterface() {
      }

      virtual bool Create(int port) = 0;

      virtual int Recv(char* buffer, int buffer_size, IpPort& from_out) = 0;
    };
  };

  class Server {
   public:
    Server();
    ~Server();

    bool Start(int port);

    void Update();

   private:
    bool started_;
    std::string buffer_;
    impl::ServerSocketInterface* sock_;
  };
};

#endif
