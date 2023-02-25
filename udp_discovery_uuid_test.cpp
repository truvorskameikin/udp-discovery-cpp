#include "udp_discovery_uuid.hpp"

#undef NDEBUG
#include <assert.h>

static bool allZeroBytes(const udpdiscovery::Uuid& uuid) {
  bool result = true;
  for (size_t i = 0; i < 16; ++i) {
    if (uuid.bytes()[i] != 0) {
      result = false;
      break;
    }
  }
  return result;
}

void uuid_nillUuid() {
  {
    udpdiscovery::Uuid nillUuid;
    assert(nillUuid.IsNill());
    assert(allZeroBytes(nillUuid));
  }

  {
    udpdiscovery::Uuid nillUuid;
    nillUuid.FromUrn("urn:uuid:00000000-0000-0000-0000-000000000000");
    assert(nillUuid.IsNill());
    assert(allZeroBytes(nillUuid));
  }
}

void uuid_fromUuid() {
  {
    udpdiscovery::Uuid nillUuid;
    udpdiscovery::Uuid newUuid(nillUuid);
    assert(newUuid.IsNill());
    assert(allZeroBytes(newUuid));
  }

  {
    udpdiscovery::Uuid oldUuid;
    oldUuid.FromUrn("urn:uuid:01234567-8901-2345-6789-abcdefabcdef");
    udpdiscovery::Uuid newUuid(oldUuid);
    assert(!newUuid.IsNill());
    assert(!allZeroBytes(newUuid));
  }
}

void uuid_fromUrn() {
  {
    udpdiscovery::Uuid uuid;
    bool result = uuid.FromUrn("urn:uuid:01234567-8901-2345-6789-abcdefabcdef");
    assert(result);
    assert(!uuid.IsNill());
    assert(uuid.bytes()[0] == 0x01);
    assert(uuid.bytes()[1] == 0x23);
    assert(uuid.bytes()[2] == 0x45);
    assert(uuid.bytes()[3] == 0x67);
    assert(uuid.bytes()[4] == 0x89);
    assert(uuid.bytes()[5] == 0x01);
    assert(uuid.bytes()[6] == 0x23);
    assert(uuid.bytes()[7] == 0x45);
    assert(uuid.bytes()[8] == 0x67);
    assert(uuid.bytes()[9] == 0x89);
    assert(uuid.bytes()[10] == 0xab);
    assert(uuid.bytes()[11] == 0xcd);
    assert(uuid.bytes()[12] == 0xef);
    assert(uuid.bytes()[13] == 0xab);
    assert(uuid.bytes()[14] == 0xcd);
    assert(uuid.bytes()[15] == 0xef);
  }

  {
    udpdiscovery::Uuid uuid;
    bool result = uuid.FromUrn("urn:uuid:01234567-8901-2345-6789-ABCDEFABCDEF");
    assert(result);
    assert(!uuid.IsNill());
    assert(uuid.bytes()[0] == 0x01);
    assert(uuid.bytes()[1] == 0x23);
    assert(uuid.bytes()[2] == 0x45);
    assert(uuid.bytes()[3] == 0x67);
    assert(uuid.bytes()[4] == 0x89);
    assert(uuid.bytes()[5] == 0x01);
    assert(uuid.bytes()[6] == 0x23);
    assert(uuid.bytes()[7] == 0x45);
    assert(uuid.bytes()[8] == 0x67);
    assert(uuid.bytes()[9] == 0x89);
    assert(uuid.bytes()[10] == 0xab);
    assert(uuid.bytes()[11] == 0xcd);
    assert(uuid.bytes()[12] == 0xef);
    assert(uuid.bytes()[13] == 0xab);
    assert(uuid.bytes()[14] == 0xcd);
    assert(uuid.bytes()[15] == 0xef);
  }
}

int main() {
  uuid_nillUuid();
  uuid_fromUuid();
  uuid_fromUrn();
}
