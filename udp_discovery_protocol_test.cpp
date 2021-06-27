#include "udp_discovery_protocol.hpp"

#undef NDEBUG
#include <assert.h>

#include <iostream>

void protocol_SerializeUnsignedIntegerBigEndian_Serialize_8() {
  uint8_t v = 0x15;
  std::string buffer;
  udpdiscovery::impl::BufferView buffer_view(&buffer);
  udpdiscovery::impl::SerializeUnsignedIntegerBigEndian(
      udpdiscovery::impl::kSerialize, &v, &buffer_view);
  assert(buffer.size() == 1);
  assert(buffer[0] == 0x15);
}

void protocol_SerializeUnsignedIntegerBigEndian_Parse_8() {
  std::string buffer("\x15");
  uint8_t v = 0;
  udpdiscovery::impl::BufferView buffer_view(&buffer);
  udpdiscovery::impl::SerializeUnsignedIntegerBigEndian(
      udpdiscovery::impl::kParse, &v, &buffer_view);
  assert(v == 0x15);
}

void protocol_SerializeUnsignedIntegerBigEndian_Serialize_16() {
  uint16_t v = 0x1516;
  std::string buffer;
  udpdiscovery::impl::BufferView buffer_view(&buffer);
  udpdiscovery::impl::SerializeUnsignedIntegerBigEndian(
      udpdiscovery::impl::kSerialize, &v, &buffer_view);
  assert(buffer.size() == 2);
  assert(buffer[0] == 0x15);
  assert(buffer[1] == 0x16);
}

void protocol_SerializeUnsignedIntegerBigEndian_Parse_16() {
  std::string buffer("\x15\x16");
  uint16_t v = 0;
  udpdiscovery::impl::BufferView buffer_view(&buffer);
  udpdiscovery::impl::SerializeUnsignedIntegerBigEndian(
      udpdiscovery::impl::kParse, &v, &buffer_view);
  assert(v == 0x1516);
}

void protocol_SerializeUnsignedIntegerBigEndian_Serialize_32() {
  uint32_t v = 0x15161718;
  std::string buffer;
  udpdiscovery::impl::BufferView buffer_view(&buffer);
  udpdiscovery::impl::SerializeUnsignedIntegerBigEndian(
      udpdiscovery::impl::kSerialize, &v, &buffer_view);
  assert(buffer.size() == 4);
  assert(buffer[0] == 0x15);
  assert(buffer[1] == 0x16);
  assert(buffer[2] == 0x17);
  assert(buffer[3] == 0x18);
}

void protocol_SerializeUnsignedIntegerBigEndian_Parse_32() {
  std::string buffer("\x15\x16\x17\x18");
  uint32_t v = 0;
  udpdiscovery::impl::BufferView buffer_view(&buffer);
  udpdiscovery::impl::SerializeUnsignedIntegerBigEndian(
      udpdiscovery::impl::kParse, &v, &buffer_view);
  assert(v == 0x15161718);
}

#pragma pack(push)
#pragma pack(1)
struct PacketHeaderV0 {
  unsigned char magic[4];
  unsigned char reserved[4];
  unsigned char packet_type;
  uint32_t application_id;
  uint32_t peer_id;
  uint64_t packet_index;
  uint16_t user_data_size;
  uint16_t padding_size;
};
#pragma pack(pop)

template <typename ValueType>
void StoreBigEndian(ValueType value, void* out) {
  unsigned char* out_typed = (unsigned char*)out;

  int n = sizeof(ValueType);
  for (int i = 0; i < n; ++i) {
    out_typed[i] = (value >> ((n - i - 1) * 8)) & 0xff;
  }
}

void protocol_Parse_withWellFormedPacketV0_readsPacket() {
  std::string user_data(
      "User data with non-printable chars: \250, \251, \252, \253, \254, \255");
  int padding_size = 100;
  std::string packet_buffer;
  packet_buffer.resize(sizeof(PacketHeaderV0) + user_data.size() +
                       padding_size);

  char* ptr = const_cast<char*>(packet_buffer.data());
  PacketHeaderV0* packet_header = (PacketHeaderV0*)ptr;
  ptr += sizeof(PacketHeaderV0);

  packet_header->magic[0] = 'R';
  packet_header->magic[1] = 'N';
  packet_header->magic[2] = '6';
  packet_header->magic[3] = 'U';
  packet_header->reserved[0] = 0;
  packet_header->reserved[1] = 0;
  packet_header->reserved[2] = 0;
  packet_header->reserved[3] = 0;
  packet_header->packet_type = udpdiscovery::kPacketIAmHere;
  StoreBigEndian<uint32_t>(12345, &packet_header->application_id);
  StoreBigEndian<uint32_t>(54321, &packet_header->peer_id);
  StoreBigEndian<uint64_t>(1234567890, &packet_header->packet_index);
  StoreBigEndian<uint16_t>(user_data.size(), &packet_header->user_data_size);
  StoreBigEndian<uint16_t>(padding_size, &packet_header->padding_size);

  for (int i = 0; i < user_data.size(); ++i) {
    *ptr = user_data[i];
    ++ptr;
  }

  udpdiscovery::Packet packet;
  assert(packet.Parse(packet_buffer) == udpdiscovery::kProtocolVersion0);
  assert(packet.packet_type() == udpdiscovery::kPacketIAmHere);
  assert(packet.application_id() == 12345);
  assert(packet.peer_id() == 54321);
  assert(packet.snapshot_index() == 1234567890);
  assert(packet.user_data() == user_data);
}

int main() {
  protocol_SerializeUnsignedIntegerBigEndian_Serialize_8();
  protocol_SerializeUnsignedIntegerBigEndian_Parse_8();
  protocol_SerializeUnsignedIntegerBigEndian_Serialize_16();
  protocol_SerializeUnsignedIntegerBigEndian_Parse_16();
  protocol_SerializeUnsignedIntegerBigEndian_Serialize_32();
  protocol_SerializeUnsignedIntegerBigEndian_Parse_32();
  protocol_Parse_withWellFormedPacketV0_readsPacket();
}
