#ifndef __UDP_DISCOVERY_NETWORK_INTERFACE_H_
#define __UDP_DISCOVERY_NETWORK_INTERFACE_H_

#include <stdint.h>
#include <string>

namespace udpdiscovery {
  class NetworkInterface {
   public:
    NetworkInterface() {
    }

    const std::string name() const {
      return name_;
    }

    void set_name(const std::string& name) {
      name_ = name;
    }

    const uint32_t broadcast_address() const {
      return broadcast_address_;
    }

    void set_broadcast_address(uint32_t broadcast_address) {
      broadcast_address_ = broadcast_address;
    }

   private:
    std::string name_;
    uint32_t broadcast_address_;
  };
}

#endif
