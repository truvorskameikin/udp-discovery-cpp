#include <stdlib.h>
#include <map>
#include <iostream>
#include "udp_discovery_peer.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

void Usage(int argc, char* argv[]) {
  std::cout << "Usage: " << argv[0] << " application_id port" << std::endl;
  std::cout << "  application_id - integer id of application to discover" << std::endl;
  std::cout << "  port - port used by application" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cerr << "expecting application_id and port" << std::endl;
    Usage(argc, argv);
    return 1;
  }

  if (argc <= 2) {
    std::cerr << "expecting port" << std::endl;
    Usage(argc, argv);
    return 1;
  }

  int port = atoi(argv[2]);
  uint64_t application_id = atoi(argv[1]);

  udpdiscovery::PeerParameters parameters;
  parameters.set_can_discover(true);
  parameters.set_can_be_discovered(false);

  parameters.set_port(port);
  parameters.set_application_id(application_id);

  udpdiscovery::Peer peer;

  if (!peer.Start(parameters, ""))
    return 1;

  std::list<udpdiscovery::DiscoveredPeer> discovered_peers;
  std::map<udpdiscovery::IpPort, std::string> last_seen_user_datas;

  while (true) {
    std::list<udpdiscovery::DiscoveredPeer> new_discovered_peers = peer.ListDiscovered();
    if (!udpdiscovery::Same(parameters.same_peer_mode(), discovered_peers, new_discovered_peers)) {
      discovered_peers = new_discovered_peers;

      last_seen_user_datas.clear();
      for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
        last_seen_user_datas.insert(std::make_pair((*it).ip_port(), (*it).user_data()));
      }

      std::cout << "Discovered peers: " << discovered_peers.size() << std::endl;
      for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
        std::cout << " - " << udpdiscovery::IpToString((*it).ip_port().ip()) << ", " << (*it).user_data() << std::endl;
      }
    } else {
      bool same_user_datas = true;
      for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = new_discovered_peers.begin(); it != new_discovered_peers.end(); ++it) {
        std::map<udpdiscovery::IpPort, std::string>::const_iterator find_it = last_seen_user_datas.find((*it).ip_port());
        if (find_it != last_seen_user_datas.end()) {
          if ((*find_it).second != (*it).user_data()) {
            same_user_datas = false;
            break;
          }
        } else {
          same_user_datas = false;
          break;
        }
      }

      if (!same_user_datas) {
        discovered_peers = new_discovered_peers;

        last_seen_user_datas.clear();
        for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
          last_seen_user_datas.insert(std::make_pair((*it).ip_port(), (*it).user_data()));
        }

        std::cout << "Discovered peers: " << discovered_peers.size() << std::endl;
        for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
          std::cout << " - " << udpdiscovery::IpToString((*it).ip_port().ip()) << ", " << (*it).user_data() << std::endl;
        }
      }
    }

#if defined(_WIN32)
    Sleep(500);
#else
    usleep(500000);
#endif
  }

  peer.Stop(true);

  return 0;
}