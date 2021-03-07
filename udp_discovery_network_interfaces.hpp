#ifndef __UDP_DISCOVERY_NETWORK_INTERFACES_H_
#define __UDP_DISCOVERY_NETWORK_INTERFACES_H_

#include <vector>
#include "udp_discovery_network_interface.hpp"

namespace udpdiscovery {
  std::vector<NetworkInterface> EnumBroadcastInterfaces();
}

#endif
