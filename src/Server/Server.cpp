#include "Server/Server.hpp"
#include "GNetworking/Socket.hpp"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include <cstdlib>
#include <iostream>
#include <openssl/types.h>

namespace AdvancedWebserver {
Server::Server(const std::string &_address, const int &_port) {
  SetupSocket(_address, _port);
  SetupSSL("domain.crt", "domain.key");
}

void Server::Run(void (*handle_func)(SSL_CTX *, GNetworking::Socket _clientSock,
                                     bool *active)) {
  bool running = true;
  GNetworking::Socket clientSock;

  while (running) {
    // Remove closed threads
    for (int i = 0; i < m_threads.size(); i++) {
      if (!m_threads[i].second) {
        m_threads[i].first->join();
        delete m_threads[i].first;
        delete m_threads[i].second;

        m_threads.erase(m_threads.begin() + i);
      }
    }

    m_serverSock.Accept(clientSock);
    m_threads.push_back(std::pair<std::thread *, bool *>());
    m_threads[m_threads.size() - 1].second = new bool(true);
    m_threads[m_threads.size() - 1].first =
        new std::thread(handle_func, m_sslContext, clientSock,
                        m_threads[m_threads.size() - 1].second);

    std::this_thread::sleep_for(std::chrono::seconds(2));
  }
}

void Server::SetupSocket(const std::string &_address, const int &_port) {
  GNetworking::Socket::Init();

  if (m_serverSock.CreateSocket(AF_INET, SOCK_STREAM, 0) < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    std::exit(EXIT_FAILURE);
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

void Server::SetupSSL(const std::string &_cert,
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

Server::~Server() {
  // Clean up used threads
  for (int i = 0; i < m_threads.size(); i++) {
    m_threads[i].first->join();
    delete m_threads[i].first;
    delete m_threads[i].second;
  }

  GNetworking::Socket::DeInit();

  SSL_CTX_free(m_sslContext);
}
} // namespace AdvancedWebserver
