#include <string.h>
#include <map>
#include <iostream>
#include "udp_discovery_peer.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

const int kPort = 12021;
const uint64_t kApplicationId = 7681412;

void Usage(int argc, char* argv[]) {
  std::cout << "Usage: " << argv[0] << " {discover|discoverable|both} [user_data]" << std::endl;
  std::cout << std::endl;
  std::cout << "discover - this instance will have the ability to only discover other instances" << std::endl;
  std::cout << "discoverable - this instance will have the ability to only be discovered by other instances" << std::endl;
  std::cout << "both - this instance will be able to discover and to be discovered by other instances" << std::endl;
  std::cout << "user_data - the string sent when broadcasting, shown next to peer's IP" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cerr << "expecting {discover|discoverable|both}" << std::endl;
    Usage(argc, argv);
    return 1;
  }

  udpdiscovery::PeerParameters parameters;
  if (strcmp(argv[1], "discover") == 0) {
    parameters.set_can_discover(true);
  } else if (strcmp(argv[1], "discoverable") == 0) {
    parameters.set_can_be_discovered(true);
  } else if (strcmp(argv[1], "both") == 0) {
    parameters.set_can_discover(true);
    parameters.set_can_be_discovered(true);
  } else {
    Usage(argc, argv);
    return 1;
  }

  std::string user_data;
  if (parameters.can_be_discovered()) {
    if (argc <= 2) {
      std::cerr << "expecting user_data" << std::endl;
      Usage(argc, argv);
      return 1;
    }

    user_data = argv[2];
  }

  parameters.set_port(kPort);
  parameters.set_application_id(kApplicationId);

  udpdiscovery::Peer peer;

  if (!peer.Start(parameters, user_data)) {
    return 1;
  }

  std::list<udpdiscovery::DiscoveredPeer> discovered_peers;
  std::map<udpdiscovery::IpPort, std::string> last_seen_user_datas;

  while (true) {
    if (parameters.can_discover()) {
      std::list<udpdiscovery::DiscoveredPeer> new_discovered_peers = peer.ListDiscovered();
      if (!udpdiscovery::Same(parameters.same_peer_mode(), discovered_peers, new_discovered_peers)) {
        discovered_peers = new_discovered_peers;

        last_seen_user_datas.clear();
        for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
          last_seen_user_datas.insert(std::make_pair((*it).ip_port(), (*it).user_data()));
        }

        std::cout << "Discovered peers: " << discovered_peers.size() << std::endl;
        for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
          std::cout << " - " << udpdiscovery::IpPortToString((*it).ip_port()) << ", " << (*it).user_data() << std::endl;
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
    } else {
      std::cout << "> ";

      std::string command;
      std::cin >> command;

      if (command == "help") {
        std::cout << "commands are: help, user_data, exit" << std::endl;
      } else if (command == "user_data") {
        std::cout << "input new user_data: ";

        std::cin >> user_data;
        peer.SetUserData(user_data);
      }

      if (command == "exit")
        break;
    }
  }

  peer.Stop(true);

  return 0;
}
