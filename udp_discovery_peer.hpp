#ifndef __UDP_DISCOVERY_PEER_H_
#define __UDP_DISCOVERY_PEER_H_

#include <list>
#include "udp_discovery_discovered_peer.hpp"
#include "udp_discovery_peer_parameters.hpp"

namespace udpdiscovery {
  namespace impl {
    class PeerEnvInterface {
     public:
      virtual ~PeerEnvInterface() {
      }

      virtual void SetUserData(const std::string& user_data) = 0;

      virtual std::list<DiscoveredPeer> ListDiscovered() = 0;

      virtual void Exit() = 0;
    };

    class MinimalisticThreadInterface {
     public:
      virtual ~MinimalisticThreadInterface() {
      }

      virtual void Detach() = 0;

      virtual void Join() = 0;
    };
  };

  class Peer {
   public:
    Peer();
    ~Peer();

    bool Start(const PeerParameters& parameters, const std::string& user_data);

    void SetUserData(const std::string& user_data);

    std::list<DiscoveredPeer> ListDiscovered() const;

    void Stop(bool wait_for_thread);

   private:
    Peer(const Peer&);
    Peer& operator=(const Peer&);

   private:
    impl::PeerEnvInterface* env_;
    impl::MinimalisticThreadInterface* thread_;
  };

  bool Same(PeerParameters::SamePeerMode mode, const IpPort& lhv, const IpPort& rhv);
  bool Same(PeerParameters::SamePeerMode mode, const std::list<DiscoveredPeer>& lhv, const std::list<DiscoveredPeer>& rhv);
};

#endif
