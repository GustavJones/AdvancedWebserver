#pragma once
#include "GNetworking/Socket.hpp"
#include <openssl/types.h>

namespace AdvancedWebserver {
void HandleConnection(SSL_CTX *_sslContext, GNetworking::Socket _clientSock,
                      bool *active);
} // namespace AdvancedWebserver
