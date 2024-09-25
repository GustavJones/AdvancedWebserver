#include "Server/Server.hpp"
#include "GNetworking/Socket.hpp"
#include <cstdlib>
#include <iostream>

namespace AdvancedWebserver {
Server::Server(const std::string &_address, const int &_port) {
  GNetworking::Socket::Init();

  if (m_serverSock.CreateSocket(AF_INET, SOCK_STREAM, 0) < 0) {
    std::cerr << "Failed to create socket" << std::endl;
    std::exit(1);
  }

  if (m_serverSock.Bind(_address, _port) < 0) {
    std::cerr << "Failed to bind socket" << std::endl;
    std::exit(1);
  }

  if (m_serverSock.Listen() < 0) {
    std::cerr << "Failed to listen on socket" << std::endl;
    std::exit(1);
  }
}

void Server::Run(void (*handle_func)(GNetworking::Socket _clientSock,
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
    m_threads[m_threads.size() - 1].first = new std::thread(
        handle_func, clientSock, m_threads[m_threads.size() - 1].second);

    std::this_thread::sleep_for(std::chrono::seconds(2));
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
}
} // namespace AdvancedWebserver
