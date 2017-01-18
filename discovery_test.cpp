#include <assert.h>
#include <iostream>
#include "protocol.hpp"

void TestProtocol() {
  std::string packet_data;
  udpdiscovery::MakePacket(udpdiscovery::kPacketIAmHere, "UserData", packet_data);

  udpdiscovery::PacketType packet_type;
  uint16_t user_data_size;
  bool result = udpdiscovery::ParsePacketHeader(
    packet_data.data(), packet_data.size(),
    packet_type, user_data_size);

  assert(result);
  assert(user_data_size == 8);
}

int main(int argc, char* argv[]) {
  TestProtocol();
  return 0;
}
