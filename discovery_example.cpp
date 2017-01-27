#include <map>
#include <iostream>
#include "endpoint.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

const int kPort = 12021;
const uint64_t kApplicationId = 5768141216339550557;

void Usage(int argc, char* argv[]) {
  std::cout << "Usage: " << argv[0] << " {discover|discoverable|both} [user_data]" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cerr << "expecting {discover|discoverable|both}" << std::endl;
    Usage(argc, argv);
    return 1;
  }

  udpdiscovery::EndpointParameters parameters;
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

  udpdiscovery::Endpoint endpoint;

  if (!endpoint.Start(parameters, user_data))
    return 1;

  std::list<udpdiscovery::DiscoveredEndpoint> discovered_endpoints;
  std::map<udpdiscovery::IpPort, std::string> last_seen_user_datas;

  while (true) {
    if (parameters.can_discover()) {
      std::list<udpdiscovery::DiscoveredEndpoint> new_discovered_endpoints = endpoint.ListDiscovered();
      if (!udpdiscovery::Same(parameters.same_endpoint_mode(), discovered_endpoints, new_discovered_endpoints)) {
        discovered_endpoints = new_discovered_endpoints;

        last_seen_user_datas.clear();
        for (std::list<udpdiscovery::DiscoveredEndpoint>::const_iterator it = discovered_endpoints.begin(); it != discovered_endpoints.end(); ++it)
          last_seen_user_datas.insert(std::make_pair((*it).ip_port(), (*it).user_data()));

        std::cout << "Discovered endpoints: " << discovered_endpoints.size() << std::endl;
        for (std::list<udpdiscovery::DiscoveredEndpoint>::const_iterator it = discovered_endpoints.begin(); it != discovered_endpoints.end(); ++it)
          std::cout << " - " << udpdiscovery::IpToString((*it).ip_port().ip()) << ", " << (*it).user_data() << std::endl;
      } else {
        bool same_user_datas = true;
        for (std::list<udpdiscovery::DiscoveredEndpoint>::const_iterator it = new_discovered_endpoints.begin(); it != new_discovered_endpoints.end(); ++it) {
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
          discovered_endpoints = new_discovered_endpoints;

          last_seen_user_datas.clear();
          for (std::list<udpdiscovery::DiscoveredEndpoint>::const_iterator it = discovered_endpoints.begin(); it != discovered_endpoints.end(); ++it)
            last_seen_user_datas.insert(std::make_pair((*it).ip_port(), (*it).user_data()));

          std::cout << "Discovered endpoints: " << discovered_endpoints.size() << std::endl;
          for (std::list<udpdiscovery::DiscoveredEndpoint>::const_iterator it = discovered_endpoints.begin(); it != discovered_endpoints.end(); ++it)
            std::cout << " - " << udpdiscovery::IpToString((*it).ip_port().ip()) << ", " << (*it).user_data() << std::endl;
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
        endpoint.SetUserData(user_data);
      }

      if (command == "exit")
        break;
    }
  }

  endpoint.Stop(true);

  return 0;
}
