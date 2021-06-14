#include "udp_discovery_protocol.hpp"

#undef NDEBUG
#include <assert.h>

void protocol_StoreBigEndian_8() {
  char buffer[1];
  udpdiscovery::impl::StoreBigEndian<uint8_t>(15, buffer);

  assert(buffer[0] == 15);
}

void protocol_StoreBigEndian_16() {
  char buffer[2];
  udpdiscovery::impl::StoreBigEndian<uint16_t>(25 + 256 * 15, buffer);

  assert(buffer[0] == 15);
  assert(buffer[1] == 25);
}

void protocol_StoreBigEndian_32() {
  char buffer[4];
  udpdiscovery::impl::StoreBigEndian<uint32_t>(
      45 + 256 * 35 + 256 * 256 * 25 + 256 * 256 * 256 * 15, buffer);

  assert(buffer[0] == 15);
  assert(buffer[1] == 25);
  assert(buffer[2] == 35);
  assert(buffer[3] == 45);
}

void protocol_StoreBigEndian_64() {
  uint64_t value = 15;
  value = value * 256 + 25;
  value = value * 256 + 35;
  value = value * 256 + 45;
  value = value * 256 + 55;
  value = value * 256 + 65;
  value = value * 256 + 75;
  value = value * 256 + 85;

  char buffer[8];
  udpdiscovery::impl::StoreBigEndian<uint64_t>(value, buffer);

  assert(buffer[0] == 15);
  assert(buffer[1] == 25);
  assert(buffer[2] == 35);
  assert(buffer[3] == 45);
  assert(buffer[4] == 55);
  assert(buffer[5] == 65);
  assert(buffer[6] == 75);
  assert(buffer[7] == 85);
}

void protocol_ReadBigEndian_8() {
  char buffer[1];
  buffer[0] = 15;
  uint8_t result = udpdiscovery::impl::ReadBigEndian<uint8_t>(buffer);

  assert(result == 15);
}

void protocol_ReadBigEndian_16() {
  char buffer[2];
  buffer[0] = 15;
  buffer[1] = 25;
  uint16_t result = udpdiscovery::impl::ReadBigEndian<uint16_t>(buffer);

  assert(result == 25 + 256 * 15);
}

void protocol_ReadBigEndian_32() {
  char buffer[4];
  buffer[0] = 15;
  buffer[1] = 25;
  buffer[2] = 35;
  buffer[3] = 45;
  uint32_t result = udpdiscovery::impl::ReadBigEndian<uint32_t>(buffer);

  assert(result == 45 + 256 * 35 + 256 * 256 * 25 + 256 * 256 * 256 * 15);
}

void protocol_ReadBigEndian_64() {
  char buffer[8];
  buffer[0] = 15;
  buffer[1] = 25;
  buffer[2] = 35;
  buffer[3] = 45;
  buffer[4] = 55;
  buffer[5] = 65;
  buffer[6] = 75;
  buffer[7] = 85;
  uint64_t result = udpdiscovery::impl::ReadBigEndian<uint64_t>(buffer);

  uint64_t value = 15;
  value = value * 256 + 25;
  value = value * 256 + 35;
  value = value * 256 + 45;
  value = value * 256 + 55;
  value = value * 256 + 65;
  value = value * 256 + 75;
  value = value * 256 + 85;

  assert(result == value);
}

void protocol_MakePacket_WithBigUserData_FailsToCreatePacket() {
  std::string user_data;
  user_data.resize(udpdiscovery::kMaxUserDataSize + 1);

  udpdiscovery::PacketHeader packet_header;
  std::string buffer_out;
  bool result =
      udpdiscovery::MakePacket(packet_header, user_data, 0, buffer_out);

  assert(result == false);
}

void protocol_MakePacket_WithBigPadding_FailsToCreatePacket() {
  std::string user_data = "user_data";

  udpdiscovery::PacketHeader packet_header;
  std::string buffer_out;
  bool result = udpdiscovery::MakePacket(
      packet_header, user_data, udpdiscovery::kMaxPaddingSize + 1, buffer_out);

  assert(result == false);
}

int main() {
  protocol_StoreBigEndian_8();
  protocol_StoreBigEndian_16();
  protocol_StoreBigEndian_32();
  protocol_StoreBigEndian_64();
  protocol_ReadBigEndian_8();
  protocol_ReadBigEndian_16();
  protocol_ReadBigEndian_32();
  protocol_ReadBigEndian_64();
  protocol_MakePacket_WithBigUserData_FailsToCreatePacket();
  protocol_MakePacket_WithBigPadding_FailsToCreatePacket();
}
