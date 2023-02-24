#ifndef __UDP_DISCOVERY_UUID_H_
#define __UDP_DISCOVERY_UUID_H_

#include <stdint.h>

#include <string>

namespace udpdiscovery {
/**
 * \brief Represents Version 4 UUID as stated in RFC 4122 with Variant 1
 * encoding (big-endian format).
 */
class Uuid {
 public:
  /**
   * \brief Constructs nill UUID.
   */
  Uuid() {
    for (size_t i = 0; i < 16; ++i) {
      uuid_[i] = 0;
    }
  }

  /**
   * \brief Constructs UUID from Uniform Resource Name (example:
   * urn:uuid:00112233-4455-6677-8899-aabbccddeeff).
   */
  bool FromUrn(const std::string& urn) {
    if (!checkFormat(
            /* s= */ urn,
            /* format= */ "urn:uuid:xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx")) {
      return false;
    }

    const char* s = urn.c_str();
    int index = 0;

    s += 9;  // "urn:uuid:"

    for (int i = 0; i < 4; ++i) {
      uuid_[index] = hexToByte(s);
      ++index;
      s += 2;
    }

    ++s;  // '-'

    for (int i = 0; i < 2; ++i) {
      uuid_[index] = hexToByte(s);
      ++index;
      s += 2;
    }

    ++s;  // '-'

    for (int i = 0; i < 2; ++i) {
      uuid_[index] = hexToByte(s);
      ++index;
      s += 2;
    }

    ++s;  // '-'

    for (int i = 0; i < 2; ++i) {
      uuid_[index] = hexToByte(s);
      ++index;
      s += 2;
    }

    ++s;  // '-'

    for (int i = 0; i < 6; ++i) {
      uuid_[index] = hexToByte(s);
      ++index;
      s += 2;
    }

    return true;
  }

  /**
   * \brief Constructs UUID from 16 bytes of Variant 1 representation.
   */
  bool FromBinary(const uint8_t* bytes, size_t bytes_size) {
    if (bytes_size < 16) {
      return false;
    }

    for (size_t i = 0; i < 16; ++i) {
      uuid_[i] = bytes[i];
    }

    return true;
  }

  /**
   * \brief Returns the Variant 1 representation of UUID.
   */
  const uint8_t* bytes() const { return uuid_; }

  bool IsNill() const {
    for (size_t i = 0; i < 16; ++i) {
      if (uuid_[i] != 0) {
        return false;
      }
    }
    return true;
  }

 private:
  static bool checkFormat(const std::string& s, const std::string& format) {
    if (s.size() < format.size()) {
      return false;
    }

    for (size_t i = 0; i < s.size(); ++i) {
      char c = s[i];
      if (format[i] == 'x') {
        bool is_good_char = ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                             (c >= 'A' && c <= 'F'));
        if (!is_good_char) {
          return false;
        }
      } else {
        if (c != format[i]) {
          return false;
        }
      }
    }

    return true;
  }

  static uint8_t hexToByte(const char* s) {
    uint8_t hi = hexToHalfByte(s[0]);
    uint8_t lo = hexToHalfByte(s[1]);
    return lo + (hi << 4);
  }

  static uint8_t hexToHalfByte(const char c) {
    if (c >= '0' && c <= '9') {
      return c - '0';
    } else if (c >= 'a' && c <= 'z') {
      return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'Z') {
      return c - 'A' + 10;
    }
    return 0;
  }

  uint8_t uuid_[16];
};
}  // namespace udpdiscovery

#endif
