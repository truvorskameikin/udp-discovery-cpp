#ifndef __UDP_DISCOVERY_PROTOCOL_H_
#define __UDP_DISCOVERY_PROTOCOL_H_

#include <stdint.h>
#include <string>

namespace udpdiscovery {
  enum PacketType {
    kPacketIAmHere,
    kPacketIAmOutOfHere
  };

#pragma pack(push)
#pragma pack(1)
  struct PacketHeader {
    PacketHeader();

    void MakeMagic();

    bool TestMagic() const;

    unsigned char magic[4];
    unsigned char packet_type;
    uint64_t application_id;
    uint64_t packet_index;
    unsigned char packet_index_reset;
    uint16_t user_data_size;
    unsigned char reserved[4];
  };
#pragma pack(pop)

  const size_t kMaxUserDataSize = 32768;
  const size_t kMaxPacketSize = 65536;

  bool MakePacket(const PacketHeader& header, const std::string& user_data, std::string& packet_data_out);

  bool ParsePacketHeader(const char* buffer, size_t buffer_size, PacketHeader& header_out);
};

#endif
