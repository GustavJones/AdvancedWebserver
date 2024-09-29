#pragma once
#include "GNetworking/Socket.hpp"
#include <openssl/ssl.h>
#include <openssl/types.h>
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

  void Run(void (*handle_func)(SSL_CTX *, GNetworking::Socket _clientSock,
                               bool *active));

private:
  SSL_CTX *m_sslContext;
  const SSL_METHOD *m_sslMethod;
  GNetworking::Socket m_serverSock;
  std::vector<std::pair<std::thread *, bool *>> m_threads;

  void SetupSocket(const std::string &_address, const int &_port);
  void SetupSSL(const std::string &_cert, const std::string &_privateKey);
};

} // namespace AdvancedWebserver
