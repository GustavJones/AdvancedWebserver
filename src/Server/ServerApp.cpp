#include "Server/ServerApp.h"
#include "GNetworking/Socket.hpp"
#include "Server/HandleConnection.h"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include <bits/types/struct_timeval.h>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <openssl/types.h>
#include <sys/socket.h>
#include <thread>

namespace AdvancedWebserver {
ServerApp::ServerApp(const std::string &_address, const int &_port,
                     const std::filesystem::path &_dataDir,
                     const std::filesystem::path &_cert,
                     const std::filesystem::path &_key)
    : m_dataDir(_dataDir) {
  SetupSocket(_address, _port);
  SetupSSL(_cert, _key);
}

void ServerApp::Run(
    void (*handle_func)(SSL_CTX *, GNetworking::Socket _clientSock,
                        const std::filesystem::path &_dataDir)) {
  constexpr const int ConnectionDelay = 10;
  bool running = true;
  GNetworking::Socket clientSock;

  // Accept connections and manage threads
  while (running) {
    if (m_serverSock.Accept(clientSock) < 0) {
      LOG("Connection Accept failed");
      std::this_thread::sleep_for(std::chrono::milliseconds(ConnectionDelay));
      continue;
    };
    std::thread t(handle_func, m_sslContext, clientSock, m_dataDir);
    t.detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(ConnectionDelay));
  }
}

void ServerApp::SetupSocket(const std::string &_address, const int &_port) {
  constexpr const bool SetRecieveTimeout = false;
  constexpr const bool SetSendTimeout = false;

  GNetworking::Socket::Init();

  if (m_serverSock.CreateSocket(AF_INET, SOCK_STREAM, 0) < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // fcntl(m_serverSock.sock, F_SETFL,
  // fcntl(m_serverSock.sock, F_GETFL, 0) | O_NONBLOCK);
  setsockopt(m_serverSock.sock, SOL_SOCKET, SO_REUSEADDR, (int *)1,
             sizeof(int));

  signal(SIGPIPE, SIG_IGN);

  if (SetRecieveTimeout) {
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (setsockopt(m_serverSock.sock, SOL_SOCKET, SO_RCVTIMEO,
                   (const char *)&tv, sizeof(tv)) < 0) {
      std::cerr << "Failed to set recieve timeout on socket" << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  if (SetSendTimeout) {
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (setsockopt(m_serverSock.sock, SOL_SOCKET, SO_SNDTIMEO,
                   (const char *)&tv, sizeof(tv)) < 0) {
      std::cerr << "Failed to set send timeout on socket" << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  if (m_serverSock.Bind(_address, _port) < 0) {
    std::cerr << "Failed to bind socket" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  if (m_serverSock.Listen() < 0) {
    std::cerr << "Failed to listen on socket" << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

void ServerApp::SetupSSL(const std::string &_cert,
                         const std::string &_privateKey) {

  m_sslMethod = TLS_server_method();
  m_sslContext = SSL_CTX_new(m_sslMethod);

  if (!m_sslContext) {
    std::cerr << "Failed to initialise SSL context" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  if (SSL_CTX_use_certificate_file(m_sslContext, _cert.c_str(),
                                   SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  if (SSL_CTX_use_PrivateKey_file(m_sslContext, _privateKey.c_str(),
                                  SSL_FILETYPE_PEM) <= 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
}

ServerApp::~ServerApp() {
  GNetworking::Socket::DeInit();

  SSL_CTX_free(m_sslContext);
}
} // namespace AdvancedWebserver
