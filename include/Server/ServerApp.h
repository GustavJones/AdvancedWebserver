#pragma once
#include "GNetworking/Socket.hpp"
#include <cstdlib>
#include <filesystem>
#include <openssl/ssl.h>
#include <openssl/types.h>
#include <string>
#include <thread>
#include <vector>

namespace AdvancedWebserver {
class ServerApp {
public:
  ServerApp(const std::string &_address, const int &_port,
            const std::filesystem::path &_dataDir);
  ServerApp(ServerApp &&) = default;
  ServerApp(const ServerApp &) = default;
  ServerApp &operator=(ServerApp &&) = default;
  ServerApp &operator=(const ServerApp &) = default;
  ~ServerApp();

  void Run(void (*handle_func)(SSL_CTX *, GNetworking::Socket _clientSock,
                               const std::filesystem::path &_dataDir));

private:
  std::filesystem::path m_dataDir;
  SSL_CTX *m_sslContext;
  const SSL_METHOD *m_sslMethod;
  GNetworking::Socket m_serverSock;
  std::vector<std::pair<std::thread *, bool *>> m_threads;

  void SetupSocket(const std::string &_address, const int &_port);
  void SetupSSL(const std::string &_cert, const std::string &_privateKey);
};

} // namespace AdvancedWebserver
