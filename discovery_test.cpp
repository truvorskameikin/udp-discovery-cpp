#include <assert.h>
#include <iostream>
#include "protocol.hpp"

void TestProtocol() {
  std::string packet_data;

  udpdiscovery::PacketHeader header;
  header.packet_type = udpdiscovery::kPacketIAmHere;
  header.packet_index = 105;
  udpdiscovery::MakePacket(header, "UserData", packet_data);

  udpdiscovery::PacketHeader header_test;
  bool result = udpdiscovery::ParsePacketHeader(
    packet_data.data(), packet_data.size(),
    header_test);

  assert(result);
  assert(header_test.packet_index == 105);
  assert(header_test.user_data_size == 8);
}

int main(int argc, char* argv[]) {
  TestProtocol();
  return 0;
}
