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
  PacketHeader::PacketHeader()
      : packet_type(kPacketIAmHere),
        packet_index(0),
        packet_index_reset(0),
        user_data_size(0) {

    MakeMagic();

    reserved[0] = 0;
    reserved[1] = 0;
    reserved[2] = 0;
    reserved[3] = 0;
  }

  void PacketHeader::MakeMagic() {
    magic[0] = 'R';
    magic[1] = 'N';
    magic[2] = '6';
    magic[3] = 'U';
  }

  bool PacketHeader::TestMagic() const {
    if (magic[0] != 'R')
      return false;
    if (magic[1] != 'N')
      return false;
    if (magic[2] != '6')
      return false;
    if (magic[3] != 'U')
      return false;
    return true;
  }

  bool MakePacket(const PacketHeader& header, const std::string& user_data, std::string& packet_data_out) {
    if (sizeof(PacketHeader) + user_data.size() > kMaxPacketSize)
      return false;

    uint16_t user_data_size = (uint16_t) user_data.size();

    packet_data_out.resize(sizeof(PacketHeader) + user_data.size());

    PacketHeader* packet_header = (PacketHeader*) packet_data_out.data();

    (*packet_header) = header;
    packet_header->MakeMagic();
    StoreBigEndian(header.packet_index, &packet_header->packet_index);
    StoreBigEndian(user_data_size, &packet_header->user_data_size);
    packet_header->reserved[0] = 0;
    packet_header->reserved[1] = 0;
    packet_header->reserved[2] = 0;
    packet_header->reserved[3] = 0;

    std::copy(
      user_data.begin(), user_data.begin() + user_data_size,
      packet_data_out.begin() + sizeof(PacketHeader));

    return true;
  }

  bool ParsePacketHeader(const char* buffer, size_t buffer_size, PacketHeader& header_out) {
    if (buffer_size < sizeof(PacketHeader))
      return false;

    const PacketHeader* header = (const PacketHeader*) buffer;
    if (!header->TestMagic())
      return false;

    if (header->packet_type != kPacketIAmHere)
      return false;

    PacketHeader parsed_packet_header = (*header);
    parsed_packet_header.packet_index = ReadBigEndian<uint64_t>(&parsed_packet_header.packet_index);
    parsed_packet_header.user_data_size = ReadBigEndian<uint16_t>(&parsed_packet_header.user_data_size);

    if (sizeof(PacketHeader) + parsed_packet_header.user_data_size > kMaxPacketSize)
      return false;

    header_out = parsed_packet_header;

    return true;
  }
}
