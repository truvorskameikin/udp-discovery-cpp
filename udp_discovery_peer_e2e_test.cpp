#include "udp_discovery_peer.hpp"

#undef NDEBUG
#include <assert.h>

#include <iostream>

const int kPort = 12021;
const uint64_t kApplicationId = 7681412;
const unsigned int kMulticastAddress =
    (224 << 24) + (0 << 16) + (0 << 8) + 123;  // 224.0.0.123

template <typename ResultType>
struct WaitResult {
  WaitResult() : is_timeout(false), has_result(false) {}

  bool is_timeout;
  bool has_result;
  ResultType result;
};

template <typename ResultType, typename Callable>
WaitResult<ResultType> Wait(long timeout, long sleep_timeout,
                            Callable callable) {
  long start_time = udpdiscovery::impl::NowTime();
  while (true) {
    WaitResult<ResultType> result = callable();
    if (result.has_result) {
      return result;
    }

    udpdiscovery::impl::SleepFor(sleep_timeout);

    long cur_time = udpdiscovery::impl::NowTime();
    if ((cur_time - start_time) > timeout) {
      break;
    }
  }

  WaitResult<ResultType> result;
  result.is_timeout = true;

  return result;
}

class FindUserDataCallable {
 public:
  FindUserDataCallable(udpdiscovery::Peer& peer, const std::string& user_data)
      : peer_(peer), user_data_(user_data) {}

  void SetUserData(const std::string& user_data) { user_data_ = user_data; }

  WaitResult<bool> operator()() {
    std::list<udpdiscovery::DiscoveredPeer> peers = peer_.ListDiscovered();
    for (std::list<udpdiscovery::DiscoveredPeer>::iterator it = peers.begin();
         it != peers.end(); ++it) {
      if ((*it).user_data() == user_data_) {
        WaitResult<bool> result;
        result.has_result = true;
        result.result = true;

        return result;
      }
    }

    return WaitResult<bool>();
  }

 private:
  udpdiscovery::Peer& peer_;
  std::string user_data_;
};

class EnsureNoUserDataCallable {
 public:
  EnsureNoUserDataCallable(udpdiscovery::Peer& peer,
                           const std::string& user_data)
      : peer_(peer), user_data_(user_data) {}

  void SetUserData(const std::string& user_data) { user_data_ = user_data; }

  WaitResult<bool> operator()() {
    std::list<udpdiscovery::DiscoveredPeer> peers = peer_.ListDiscovered();
    for (std::list<udpdiscovery::DiscoveredPeer>::iterator it = peers.begin();
         it != peers.end(); ++it) {
      if ((*it).user_data() == user_data_) {
        return WaitResult<bool>();
      }
    }

    WaitResult<bool> result;
    result.has_result = true;
    result.result = true;

    return result;
  }

 private:
  udpdiscovery::Peer& peer_;
  std::string user_data_;
};

void peer_udp_broadcast_discovery() {
  udpdiscovery::PeerParameters peer_parameters;
  peer_parameters.set_can_discover(true);
  peer_parameters.set_can_be_discovered(true);
  peer_parameters.set_port(kPort);
  peer_parameters.set_application_id(kApplicationId);
  peer_parameters.set_send_timeout_ms(100);

  udpdiscovery::Peer peer1;
  peer1.Start(peer_parameters, "peer 1");

  // TODO: Understand why do we need this timeout.
  udpdiscovery::impl::SleepFor(1000);

  udpdiscovery::Peer peer2;
  peer2.Start(peer_parameters, "peer 2");

  FindUserDataCallable find_peer2(peer1, "peer 2");
  WaitResult<bool> find1 =
      Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                 /* callable= */ find_peer2);
  assert(find1.is_timeout == false);
  assert(find1.has_result == true);

  FindUserDataCallable find_peer1(peer2, "peer 1");
  WaitResult<bool> find2 =
      Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                 /* callable= */ find_peer1);
  assert(find2.is_timeout == false);
  assert(find2.has_result == true);

  peer1.StopAndWaitForThreads();
  peer2.StopAndWaitForThreads();
}

void peer_udp_multicast_discovery() {
  udpdiscovery::PeerParameters peer_parameters;
  peer_parameters.set_can_discover(true);
  peer_parameters.set_can_be_discovered(true);
  peer_parameters.set_port(kPort);
  peer_parameters.set_application_id(kApplicationId);
  peer_parameters.set_send_timeout_ms(100);
  peer_parameters.set_can_use_broadcast(false);
  peer_parameters.set_can_use_multicast(true);
  peer_parameters.set_multicast_group_address(kMulticastAddress);

  udpdiscovery::Peer peer1;
  peer1.Start(peer_parameters, "peer 1");

  // TODO: Understand why do we need this timeout.
  udpdiscovery::impl::SleepFor(1000);

  udpdiscovery::Peer peer2;
  peer2.Start(peer_parameters, "peer 2");

  FindUserDataCallable find_peer2(peer1, "peer 2");
  WaitResult<bool> find1 =
      Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                 /* callable= */ find_peer2);
  assert(find1.is_timeout == false);
  assert(find1.has_result == true);

  FindUserDataCallable find_peer1(peer2, "peer 1");
  WaitResult<bool> find2 =
      Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                 /* callable= */ find_peer1);
  assert(find2.is_timeout == false);
  assert(find2.has_result == true);

  peer1.StopAndWaitForThreads();
  peer2.StopAndWaitForThreads();
}

void peer_change_user_data() {
  udpdiscovery::PeerParameters peer_parameters;
  peer_parameters.set_can_discover(true);
  peer_parameters.set_can_be_discovered(true);
  peer_parameters.set_port(kPort);
  peer_parameters.set_application_id(kApplicationId);
  peer_parameters.set_send_timeout_ms(100);
  peer_parameters.set_multicast_group_address(kMulticastAddress);

  udpdiscovery::Peer peer1;
  peer1.Start(peer_parameters, "peer 1");

  // TODO: Understand why do we need this timeout.
  udpdiscovery::impl::SleepFor(1000);

  udpdiscovery::Peer peer2;
  peer2.Start(peer_parameters, "peer 2");

  FindUserDataCallable find_peer2(peer1, "peer 2");
  WaitResult<bool> find =
      Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                 /* callable= */ find_peer2);
  assert(find.is_timeout == false);
  assert(find.has_result == true);

  peer2.SetUserData("peer 2 updated user data");

  find_peer2.SetUserData("peer 2 updated user data");
  find = Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                    /* callable= */ find_peer2);
  assert(find.is_timeout == false);
  assert(find.has_result == true);

  peer1.StopAndWaitForThreads();
  peer2.StopAndWaitForThreads();
}

void peer_disappear() {
  udpdiscovery::PeerParameters peer_parameters;
  peer_parameters.set_can_discover(true);
  peer_parameters.set_can_be_discovered(true);
  peer_parameters.set_port(kPort);
  peer_parameters.set_application_id(kApplicationId);
  peer_parameters.set_send_timeout_ms(100);
  peer_parameters.set_multicast_group_address(kMulticastAddress);

  udpdiscovery::Peer peer1;
  peer1.Start(peer_parameters, "peer 1");

  // TODO: Understand why do we need this timeout.
  udpdiscovery::impl::SleepFor(1000);

  udpdiscovery::Peer peer2;
  peer2.Start(peer_parameters, "peer 2");

  FindUserDataCallable find_peer2(peer1, "peer 2");
  WaitResult<bool> wait_result =
      Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                 /* callable= */ find_peer2);
  assert(wait_result.is_timeout == false);
  assert(wait_result.has_result == true);

  peer2.StopAndWaitForThreads();

  EnsureNoUserDataCallable no_peer2(peer1, "peer 2");
  wait_result = Wait<bool>(/* timeout = */ 5000, /* sleep_timeout = */ 200,
                           /* callable= */ no_peer2);

  assert(wait_result.is_timeout == false);
  assert(wait_result.has_result == true);

  peer1.StopAndWaitForThreads();
}

int main() {
  peer_udp_broadcast_discovery();
  peer_udp_multicast_discovery();
  peer_change_user_data();
  peer_disappear();
  return 0;
}
