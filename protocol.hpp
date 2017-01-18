#ifndef __UDP_DISCOVERY_PROTOCOL_H_
#define __UDP_DISCOVERY_PROTOCOL_H_

#include <stdint.h>
#include <string>

namespace udpdiscovery {
  enum PacketType {
    kPacketIAmHere
  };

#pragma pack(push)
#pragma pack(1)
  struct Header {
    unsigned char magic[4];
    unsigned char packet_type;
    uint16_t user_data_size;
    unsigned char reserved[4];
  };
#pragma pack(pop)

  void MakePacket(PacketType type, const std::string& user_data, std::string& packet_data);

  bool ParsePacketHeader(const char* buffer, size_t buffer_size, PacketType& type_out, uint16_t& user_data_size_out);
};

#endif
