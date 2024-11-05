#pragma once
#include "GNetworking/Socket.hpp"
#include "GParsing/HTTP/HTTPRequest.hpp"
#include "GParsing/HTTP/HTTPResponse.hpp"
#include <ctime>
#include <filesystem>
#include <openssl/types.h>
#include <vector>

namespace AdvancedWebserver {
void HandleConnection(SSL_CTX *_sslContext, GNetworking::Socket _clientSock,
                      bool *active, const std::filesystem::path &_dataDir);

bool HandleRequest(SSL *_ssl);

bool HasHeaderWithValue(const GParsing::HTTPRequest &_req,
                        const std::string &_header,
                        const std::string &_value = "");

bool HasHeaderWithValue(const GParsing::HTTPResponse &_req,
                        const std::string &_header,
                        const std::string &_value = "");

bool HasHostHeader(const GParsing::HTTPRequest &_req);

void SendHostHeaderErrorResponse(SSL *_ssl);

void SendContinueResponse(SSL *_ssl);

bool SendGetResponse(SSL *_ssl, const GParsing::HTTPRequest &_req);

int SendBuffer(SSL *_ssl, std::vector<unsigned char> _buff);

std::string GetCurrentDate();

std::time_t ParseDate(const std::string &_time);
} // namespace AdvancedWebserver
