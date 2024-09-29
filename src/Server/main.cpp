#include "Server/HandleConnection.hpp"
#include "Server/Server.hpp"
#include <iostream>

static constexpr const char ADDRESS[] = "0.0.0.0";
static constexpr const int PORT = 8081;

int main(int argc, char *argv[]) {
  AdvancedWebserver::Server server(ADDRESS, PORT);
  std::cout << "Starting server on: " << ADDRESS << ':' << PORT << std::endl;
  server.Run(AdvancedWebserver::HandleConnection);
  return 0;
}
