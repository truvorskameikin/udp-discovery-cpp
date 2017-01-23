#include <sstream>
#include "ip_port.hpp"

namespace udpdiscovery {
  void IpPort::set_ip(unsigned int ip) {
    std::stringstream ss;
    ss << ((ip >> 24) & 0xff) << "." << ((ip >> 16) & 0xff) << "." << ((ip >> 8) & 0xff) << "." << ((ip >> 0) & 0xff);

    ip_ = ss.str();
  }
};
