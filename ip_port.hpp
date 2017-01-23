#ifndef __DISCOVERY_IP_PORT_H_
#define __DISCOVERY_IP_PORT_H_

#include <string>

namespace udpdiscovery {
  class IpPort {
   public:
    IpPort() : ip_("127.0.0.1"), port_(80) {
    }

    IpPort(const std::string& ip, int port) : ip_(ip), port_(port) {
    }

    IpPort(unsigned int ip, int port) : port_(port) {
      set_ip(ip);
    }

    void set_ip(const std::string& ip) {
      ip_ = ip;
    }

    void set_ip(unsigned int ip);

    const std::string& ip() const {
      return ip_;
    }

    void set_port(int port) {
      port_ = port;
    }

    int port() const {
      return port_;
    }

   private:
    std::string ip_;
    int port_;
  };
};

#endif
