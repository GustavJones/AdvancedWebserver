#include "GNetworking/Socket.hpp"
#include "Server/Server.hpp"
#include <iostream>

void HandleConnection(GNetworking::Socket _clientSock, bool *active) {
  std::string buffer;

  while (true) {
    buffer = "";
    _clientSock.Recv(buffer);

    if (buffer == "") {
      break;
    } else {
      std::cout << buffer;
      _clientSock.Send(buffer);
    }
  }

  _clientSock.Close();
  *active = false;
}

int main(int argc, char *argv[]) {
  AdvancedWebserver::Server server("0.0.0.0", 8081);
  server.Run(HandleConnection);
  return 0;
}
