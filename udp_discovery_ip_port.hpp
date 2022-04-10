#ifndef __UDP_DISCOVERY_IP_PORT_H_
#define __UDP_DISCOVERY_IP_PORT_H_

#include <string>

namespace udpdiscovery {
  class IpPort {
   public:
    IpPort() : ip_(0), port_(0) {
    }

    IpPort(unsigned int ip, int port) : ip_(ip), port_(port) {
    }

    void set_ip(unsigned int ip) {
      ip_ = ip;
    }

    unsigned int ip() const {
      return ip_;
    }

    void set_port(int port) {
      port_ = port;
    }

    int port() const {
      return port_;
    }

    bool operator==(const IpPort& rhv) const {
      return ip_ == rhv.ip_ && port_ == rhv.port_;
    }

    bool operator<(const IpPort& rhv) const {
      if (ip_ < rhv.ip_)
        return true;
      else if (ip_ > rhv.ip_)
        return false;
      return port_ < rhv.port_;
    }

   private:
    unsigned int ip_;
    int port_;
  };

  std::string IpToString(unsigned int ip);

  std::string IpPortToString(const IpPort& ip_port);
}

#endif
