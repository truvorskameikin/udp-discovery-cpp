#include "udp_discovery_network_interfaces.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#endif

namespace udpdiscovery {
  std::vector<NetworkInterface> EnumBroadcastInterfaces() {
    std::vector<NetworkInterface> result;
#if defined(_WIN32)
    ULONG buffer_size = 0;
    ULONG win_result = GetAdaptersInfo(0, &buffer_size);
    if (win_result != ERROR_BUFFER_OVERFLOW || buffer_size == 0) {
      return result;
    }

    std::vector<char> buffer(buffer_size);
    PIP_ADAPTER_INFO current_adapter = (PIP_ADAPTER_INFO) &buffer[0];

    win_result = GetAdaptersInfo(current_adapter, &buffer_size);
    if (win_result != NO_ERROR) {
      return result;
    }

    while (current_adapter) {
      result.push_back(NetworkInterface());
      result.back().set_name(current_adapter->AdapterName);

      uint32_t ip;
      inet_pton(AF_INET, current_adapter->IpAddressList.IpAddress.String, &ip);

      uint32_t mask;
      inet_pton(AF_INET, current_adapter->IpAddressList.IpMask.String, &mask);

      result.back().set_broadcast_address(ntohl(ip | ~mask));

      current_adapter = current_adapter->Next;
    }
#endif

    return result;
  }
}
