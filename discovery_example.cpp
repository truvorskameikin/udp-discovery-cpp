#include <iostream>
#include "server.hpp"
#include "client.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

const int PORT = 12021;

void Usage(int argc, char* argv[]) {
  std::cout << "Usage: " << argv[0] << " {server|client|both} [user_data]" << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    Usage(argc, argv);
    return 1;
  }

  bool start_server = false;
  bool start_client = false;
  if (strcmp(argv[1], "server") == 0) {
    start_server = true;
  } else if (strcmp(argv[1], "client") == 0) {
    start_client = true;
  } else if (strcmp(argv[1], "both") == 0) {
    start_server = true;
    start_client = true;
  } else {
    Usage(argc, argv);
    return 1;
  }

  udpdiscovery::Server server;
  if (start_server) {
    if (!server.Start(PORT))
      return 1;
  }

  udpdiscovery::Client client;
  if (start_client) {
    if (argc <= 2) {
      Usage(argc, argv);
      return 1;
    }

    std::string user_data(argv[2]);
    client.Start(PORT, user_data);
  }

  std::list<udpdiscovery::DiscoveredClient> clients;
  while (true) {
    if (start_server) {
      std::list<udpdiscovery::DiscoveredClient> new_clients = server.ListClients();
      if (!udpdiscovery::Same(clients, new_clients)) {
        clients = new_clients;

        std::cout << "Discovered clients: " << clients.size() << std::endl;
        for (std::list<udpdiscovery::DiscoveredClient>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
          std::cout << " - " << (*it).ip_port().ip() << ", " << (*it).user_data() << std::endl;
        }
      }
    }

#if defined(_WIN32)
    Sleep(500);
#else
    usleep(500000);
#endif
  }

  return 0;
}
