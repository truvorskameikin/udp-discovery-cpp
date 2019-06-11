#ifndef __UDP_DISCOVERY_PEER_PARAMETERS_H_
#define __UDP_DISCOVERY_PEER_PARAMETERS_H_

#include <stdint.h>

namespace udpdiscovery {
  class PeerParameters {
   public:
    enum SamePeerMode {
      kSamePeerIp,
      kSamePeerIpAndPort
    };

   public:
    PeerParameters()
        : port_(0),
          application_id_(0),
          receive_timeout_ms_(200),
          send_timeout_ms_(5000),
          discovered_peer_ttl_ms_(10000),
          can_be_discovered_(false),
          can_discover_(false),
          discover_self_(false),
          same_peer_mode_(kSamePeerIpAndPort) {
    }

    int port() const {
      return port_;
    }

    void set_port(int port) {
      port_ = port;
    }

    uint32_t application_id() const {
      return application_id_;
    }

    void set_application_id(uint32_t application_id) {
      application_id_ = application_id;
    }

    int receive_timeout_ms() const {
      return receive_timeout_ms_;
    }

    void set_receive_timeout_ms(int receive_timeout_ms) {
      if (receive_timeout_ms < 0)
        return;
      receive_timeout_ms_ = receive_timeout_ms;
    }

    int send_timeout_ms() const {
      return send_timeout_ms_;
    }

    void set_send_timeout_ms(int send_timeout_ms) {
      if (send_timeout_ms < 0)
        return;
      send_timeout_ms_ = send_timeout_ms;
    }

    int discovered_peer_ttl_ms() const {
      return discovered_peer_ttl_ms_;
    }

    void set_discovered_peer_ttl_ms(int discovered_peer_ttl_ms) {
      if (discovered_peer_ttl_ms < 0)
        return;
      discovered_peer_ttl_ms_ = discovered_peer_ttl_ms;
    }

    bool can_be_discovered() const {
      return can_be_discovered_;
    }

    void set_can_be_discovered(bool can_be_discovered) {
      can_be_discovered_ = can_be_discovered;
    }

    bool can_discover() const {
      return can_discover_;
    }

    void set_can_discover(bool can_discover) {
      can_discover_ = can_discover;
    }

    bool discover_self() const {
      return discover_self_;
    }

    void set_discover_self(bool discover_self) {
      discover_self_ = discover_self;
    }

    SamePeerMode same_peer_mode() const {
      return same_peer_mode_;
    }

    void set_same_peer_mode(SamePeerMode same_peer_mode) {
      same_peer_mode_ = same_peer_mode;
    }

   private:
    int port_;
    uint32_t application_id_;
    int receive_timeout_ms_;
    int send_timeout_ms_;
    int discovered_peer_ttl_ms_;
    bool can_be_discovered_;
    bool can_discover_;
    bool discover_self_;
    SamePeerMode same_peer_mode_;
  };
};

#endif
