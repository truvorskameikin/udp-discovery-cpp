#include <sstream>
#include "udp_discovery_ip_port.hpp"

namespace udpdiscovery {
  std::string IpToString(unsigned int ip) {
    std::stringstream ss;
    ss << ((ip >> 24) & 0xff) << "." << ((ip >> 16) & 0xff) << "." << ((ip >> 8) & 0xff) << "." << ((ip >> 0) & 0xff);

    return ss.str();
  }

  std::string IpPortToString(const IpPort& ip_port) {
    std::string ip_string = IpToString(ip_port.ip());

    std::stringstream ss;
    ss << ip_port.port();

    return ip_string + ":" + ss.str();
  }
};
