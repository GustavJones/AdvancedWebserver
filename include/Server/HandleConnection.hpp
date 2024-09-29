#pragma once
#include "GNetworking/Socket.hpp"
#include "GParsing/HTTP/HTTPRequest.hpp"
#include <openssl/types.h>

namespace AdvancedWebserver {
void HandleConnection(SSL_CTX *_sslContext, GNetworking::Socket _clientSock,
                      bool *active);

bool HandleRequest(SSL *_ssl);

bool HasHostHeader(const GParsing::HTTPRequest &_req);

void SendHostHeaderErrorResponse(SSL *_ssl);

void SendContinueResponse(SSL *_ssl);

void SendGetResponse(SSL *_ssl, const GParsing::HTTPRequest &_req);
} // namespace AdvancedWebserver
