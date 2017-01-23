#include <iostream>
#include "thread_routines.h"
#include "server.hpp"
#include "client.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

void ClientFunc(std::string user_data) {
  udpdiscovery::Client client;
  if (client.Start(12012, user_data))
    std::cout << "Client started" << std::endl;

  while (true) {
    client.Send();

#if defined(_WIN32)
    Sleep(1000);
#else
    usleep(1000000);
#endif
  }
}

int main(int argc, char* argv[]) {
  udpdiscovery::Server server;
  if (!server.Start(12012)) {
    std::cerr << "Can't start server" << std::endl;
    return 1;
  }

  std::string user_data(argv[1]);
  threadroutines::thread client_thread(&ClientFunc, std::move(user_data));

  std::list<udpdiscovery::DiscoveredClient> clients;
  while (true) {
    std::list<udpdiscovery::DiscoveredClient> new_clients = server.ListClients();
    if (!udpdiscovery::Same(clients, new_clients)) {
      clients = new_clients;

      std::cout << "Discovered clients: " << clients.size() << std::endl;
      for (std::list<udpdiscovery::DiscoveredClient>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        std::cout << " - " << (*it).ip_port().ip() << ", " << (*it).user_data() << std::endl;
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
