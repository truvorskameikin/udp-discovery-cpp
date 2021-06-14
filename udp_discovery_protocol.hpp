#ifndef __UDP_DISCOVERY_PROTOCOL_H_
#define __UDP_DISCOVERY_PROTOCOL_H_

#include <stdint.h>

#include <string>

namespace udpdiscovery {
namespace impl {
template <typename ValueType>
void StoreBigEndian(ValueType value, void* out) {
  unsigned char* out_typed = (unsigned char*)out;

  const size_t n = sizeof(ValueType);
  for (size_t i = 0; i < n; ++i)
    out_typed[i] = (value >> ((n - i - 1) * 8)) & 0xff;
}

template <typename ValueType>
ValueType ReadBigEndian(const void* in) {
  const unsigned char* in_typed = (const unsigned char*)in;

  ValueType result = 0;

  const size_t n = sizeof(ValueType);
  for (size_t i = 0; i < n; ++i) {
    ValueType v = in_typed[i];
    result |= (v << ((n - i - 1) * 8));
  }

  return result;
}
}  // namespace impl

enum PacketType { kPacketIAmHere, kPacketIAmOutOfHere };

inline bool IsKnownPacketType(PacketType packet_type) {
  switch (packet_type) {
    case kPacketIAmHere:
      return true;
    case kPacketIAmOutOfHere:
      return true;
  }

  return false;
}

typedef uint64_t PacketIndex;

#pragma pack(push)
#pragma pack(1)
struct PacketHeader {
  PacketHeader();

  void MakeMagic();

  bool TestMagic() const;

  unsigned char magic[4];
  unsigned char reserved[4];
  unsigned char packet_type;
  uint32_t application_id;
  uint32_t peer_id;
  PacketIndex packet_index;
  uint16_t user_data_size;
  uint16_t padding_size;
};
#pragma pack(pop)

const size_t kMaxUserDataSize = 32768;
const size_t kMaxPaddingSize = 32768;
const size_t kMaxPacketSize = 65536;

bool MakePacket(const PacketHeader& header, const std::string& user_data,
                size_t padding_size, std::string& packet_data_out);

inline bool MakePacket(const PacketHeader& header, const std::string& user_data,
                       std::string& packet_data_out) {
  return MakePacket(header, user_data, 0, packet_data_out);
}

bool ParsePacketHeader(const char* buffer, size_t buffer_size,
                       PacketHeader& header_out, const char*& buffer_left_out,
                       size_t& buffer_left_size_out);

bool ReadUserData(const char* buffer, size_t buffer_size,
                  const PacketHeader& header, std::string& user_data_out,
                  const char*& buffer_left_out, size_t& buffer_left_size_out);

bool ReadPadding(const char* buffer, size_t buffer_size,
                 const PacketHeader& header, const char*& buffer_left_out,
                 size_t& buffer_left_size_out);

inline bool ParsePacket(const char* buffer, size_t buffer_size,
                        PacketHeader& header_out, std::string& user_data_out) {
  PacketHeader header;
  const char* buffer_left = 0;
  size_t buffer_left_size = 0;

  if (!ParsePacketHeader(buffer, buffer_size, header, buffer_left,
                         buffer_left_size)) {
    return false;
  }

  std::string user_data;
  if (!ReadUserData(buffer_left, buffer_left_size, header, user_data,
                    buffer_left, buffer_left_size)) {
    return false;
  }

  if (!ReadPadding(buffer_left, buffer_left_size, header, buffer_left,
                   buffer_left_size)) {
    return false;
  }

  header_out = header;
  std::swap(user_data_out, user_data);

  return true;
}
};  // namespace udpdiscovery

#endif
