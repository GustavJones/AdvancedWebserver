#pragma once
#include "GNetworking/Socket.hpp"
#include <string>
#include <thread>
#include <vector>

namespace AdvancedWebserver {
class Server {
public:
  Server(const std::string &_address, const int &_port);
  Server(Server &&) = default;
  Server(const Server &) = default;
  Server &operator=(Server &&) = default;
  Server &operator=(const Server &) = default;
  ~Server();

  void Run(void (*handle_func)(GNetworking::Socket _clientSock, bool *active));

private:
  GNetworking::Socket m_serverSock;
  std::vector<std::pair<std::thread *, bool *>> m_threads;
};

} // namespace AdvancedWebserver
