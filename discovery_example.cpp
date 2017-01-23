#include <iostream>
#include "thread_routines.h"
#include "server.hpp"
#include "client.hpp"

void ServerFunc() {
  udpdiscovery::Server server;
  if (server.Start(12345))
    std::cout << "Server started" << std::endl;

  while (true) {
    server.Update();
  }
};

void ClientFunc(std::string user_data) {
  udpdiscovery::Client client;
  if (client.Start(12345, user_data))
    std::cout << "Client started" << std::endl;

  while (true) {
    threadroutines::this_thread::sleep_for(1.0);

    client.Send();
  }
}

int main(int argc, char* argv[]) {
  threadroutines::thread server_thread(&ServerFunc);

  std::string user_data1("User 1");
  threadroutines::thread client_thread1(&ClientFunc, std::move(user_data1));

  std::string user_data2("Long user data");
  threadroutines::thread client_thread2(&ClientFunc, std::move(user_data2));

  server_thread.join();
  client_thread1.join();
  client_thread2.join();

  return 0;
}
