#include "udp_discovery_protocol.hpp"

#undef NDEBUG
#include <assert.h>

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

const uint32_t kApplicationId = 12345;
const uint32_t kPeerId = 54321;
const uint64_t kSnapshotIndex = 1234567890;

std::string CreatePacketV0(const std::string& user_data, size_t padding_size) {
  std::string result;
  result.resize(sizeof(PacketHeaderV0) + user_data.size() + padding_size);

  char* ptr = const_cast<char*>(result.data());
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
  StoreBigEndian<uint32_t>(kApplicationId, &packet_header->application_id);
  StoreBigEndian<uint32_t>(kPeerId, &packet_header->peer_id);
  StoreBigEndian<uint64_t>(kSnapshotIndex, &packet_header->packet_index);
  StoreBigEndian<uint16_t>(user_data.size(), &packet_header->user_data_size);
  StoreBigEndian<uint16_t>(padding_size, &packet_header->padding_size);

  for (int i = 0; i < user_data.size(); ++i) {
    *ptr = user_data[i];
    ++ptr;
  }

  return result;
}

void protocol_Parse_withWellFormedPacketV0_readsPacket() {
  std::string user_data(
      "User data with non-printable chars: \250, \251, \252, \253, \254, \255");
  std::string packet_buffer = CreatePacketV0(user_data, 100);

  udpdiscovery::Packet packet;
  assert(packet.Parse(packet_buffer) == udpdiscovery::kProtocolVersion0);
  assert(packet.packet_type() == udpdiscovery::kPacketIAmHere);
  assert(packet.application_id() == kApplicationId);
  assert(packet.peer_id() == kPeerId);
  assert(packet.snapshot_index() == kSnapshotIndex);
  assert(packet.user_data() == user_data);
}

void protocol_Parse_withTooBigUserDataV0_failsToReadPacket() {
  std::string user_data;
  user_data.resize(udpdiscovery::kMaxUserDataSizeV0 + 1);
  std::string packet_buffer = CreatePacketV0(user_data, 100);

  udpdiscovery::Packet packet;
  assert(packet.Parse(packet_buffer) == udpdiscovery::kProtocolVersionUnknown);
}

void protocol_Parse_withTooBigPaddingV0_failsToReadPacket() {
  std::string user_data("user data");
  std::string packet_buffer =
      CreatePacketV0(user_data, udpdiscovery::kMaxPaddingSizeV0 + 1);

  udpdiscovery::Packet packet;
  assert(packet.Parse(packet_buffer) == udpdiscovery::kProtocolVersionUnknown);
}

std::string CreatePacketV1(const std::string& user_data) {
  std::string result;

  udpdiscovery::Packet packet;
  packet.set_packet_type(udpdiscovery::kPacketIAmHere);
  packet.set_application_id(kApplicationId);
  packet.set_peer_id(kPeerId);
  packet.set_snapshot_index(kSnapshotIndex);
  packet.set_user_data(user_data);

  packet.Serialize(udpdiscovery::kProtocolVersion1, result);

  return result;
}

void protocol_Serialize_Parse_V1() {
  std::string user_data("user data");
  std::string packet_buffer = CreatePacketV1(user_data);

  udpdiscovery::Packet packet;
  assert(packet.Parse(packet_buffer) == udpdiscovery::kProtocolVersion1);
  assert(packet.packet_type() == udpdiscovery::kPacketIAmHere);
  assert(packet.application_id() == kApplicationId);
  assert(packet.peer_id() == kPeerId);
  assert(packet.snapshot_index() == kSnapshotIndex);
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
  protocol_Parse_withTooBigUserDataV0_failsToReadPacket();
  protocol_Parse_withTooBigPaddingV0_failsToReadPacket();
  protocol_Serialize_Parse_V1();
}
