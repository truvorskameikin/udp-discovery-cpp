#include <algorithm>
#include "protocol.hpp"

template<typename ValueType>
void StoreBigEndian(ValueType value, void* out) {
  unsigned char* out_typed = (unsigned char*) out;

  const size_t n = sizeof(ValueType);
  for (size_t i = 0; i < n; ++i)
    out_typed[i] = (value >> ((n - i - 1) * 8)) & 0xff;
}

template<typename ValueType>
ValueType ReadBigEndian(const void* in) {
  const unsigned char* in_typed = (const unsigned char*) in;

  ValueType result = 0;

  const size_t n = sizeof(ValueType);
  for (size_t i = 0; i < n; ++i) {
    ValueType v = in_typed[ i ];
    result |= (v << ((n - i - 1) * 8));
  }

  return result;
}

namespace udpdiscovery {
  void MakePacket(PacketType packet_type, const std::string& user_data, std::string& packet_data_out) {
    uint16_t user_data_size = (uint16_t) user_data.size();

    packet_data_out.resize(sizeof(Header) + user_data.size());

    Header* header = (Header*) packet_data_out.data();
    header->magic[0] = 'R';
    header->magic[1] = 'N';
    header->magic[2] = '6';
    header->magic[3] = 'U';
    header->packet_type = packet_type;
    StoreBigEndian(user_data_size, &header->user_data_size);
    header->reserved[0] = 0;
    header->reserved[1] = 0;
    header->reserved[2] = 0;
    header->reserved[3] = 0;

    std::copy(
      user_data.begin(), user_data.begin() + user_data_size,
      packet_data_out.begin() + sizeof(Header));
  }

  bool ParsePacketHeader(const char* buffer, size_t buffer_size, PacketType& type_out, uint16_t& user_data_size_out) {
    if (buffer_size < sizeof(Header))
      return false;

    const Header* header = (const Header*) buffer;
    if (header->magic[0] != 'R')
      return false;
    if (header->magic[1] != 'N')
      return false;
    if (header->magic[2] != '6')
      return false;
    if (header->magic[3] != 'U')
      return false;

    if (header->packet_type != kPacketIAmHere)
      return false;

    type_out = (PacketType) header->packet_type;
    user_data_size_out = ReadBigEndian<uint16_t>(&header->user_data_size);

    return true;
  }
}
