#include "Server/HandleConnection.hpp"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include "GParsing/HTTP/HTTPMethod.hpp"
#include "GParsing/HTTP/HTTPRequest.hpp"
#include "GParsing/HTTP/HTTPResponse.hpp"
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/types.h>
#include <string>
#include <utility>
#include <vector>

namespace AdvancedWebserver {

void HandleConnection(SSL_CTX *_sslContext, GNetworking::Socket _clientSock,
                      bool *active) {
  bool handling_requests = true;
  SSL *ssl;

  ssl = SSL_new(_sslContext);
  SSL_set_fd(ssl, _clientSock.sock);
  SSL_accept(ssl);

  while (handling_requests) {
    handling_requests = HandleRequest(ssl);
  }

  SSL_shutdown(ssl);
  SSL_free(ssl);

  *active = false;
}

bool HandleRequest(SSL *_ssl) {
  GParsing::HTTPRequest req;
  constexpr const int BUFFER_LENGTH = 2048;
  int bufferReadLen;
  bool hasHostHeader = false;

  char *buffer = new char[BUFFER_LENGTH]();
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    buffer[i] = 0;
  }

  // Accept Requests
  bufferReadLen = SSL_read(_ssl, buffer, BUFFER_LENGTH);

  // Break if connection ends
  if (buffer[0] == 0) {
    return false;
  }

  // Parse Request
  try {
    req.ParseRequest(GParsing::ConvertToCharArray(buffer, bufferReadLen));

  } catch (const std::exception &e) {
    std::cerr << "GParsing error" << ": " << e.what() << std::endl;
    return false;
  }

  hasHostHeader = HasHostHeader(req);

  if (!hasHostHeader && req.version == "HTTP/1.1") {
    SendHostHeaderErrorResponse(_ssl);
    return false;
  }

  SendContinueResponse(_ssl);

  // Handle Requests
  switch (req.method) {
  case GParsing::HTTPMethod::GET: {
    SendGetResponse(_ssl, req);
    break;
  }

  default:
    break;
  }

  delete[] buffer;
  return true;
}

bool HasHostHeader(const GParsing::HTTPRequest &_req) {
  bool hasHostHeader = false;

  // Check for invalid requests
  for (const auto &header : _req.headers) {
    if (header.first == "Host") {
      hasHostHeader = true;
    }
  }

  return hasHostHeader;
}

void SendHostHeaderErrorResponse(SSL *_ssl) {
  GParsing::HTTPResponse resp;
  std::vector<unsigned char> respVector;
  char *respBuffer;
  std::string msg;

  msg = "<html><body>\r\n<h2>No Host: header received</h2>\r\nHTTP 1.1 "
        "requests must include the Host: header.\r\n</body></html>";

  resp.version = "HTTP/1.1";
  resp.response_code = 400;
  resp.response_code_message = "Bad Request";
  resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Content-Type", {"text/html"}));
  resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Content-Length", {std::to_string(msg.length())}));

  resp.message = GParsing::ConvertToCharArray(msg.c_str(), msg.length());

  respVector = resp.CreateResponse();
  respBuffer = new char[respVector.size()]();

  for (int i = 0; i < respVector.size(); i++) {
    respBuffer[i] = respVector[i];
  }

  SSL_write(_ssl, respBuffer, respVector.size());
  delete[] respBuffer;
}

void SendContinueResponse(SSL *_ssl) {
  GParsing::HTTPResponse resp;
  std::vector<unsigned char> respVector;
  char *respBuffer;

  resp.version = "HTTP/1.1";
  resp.response_code = 100;
  resp.response_code_message = "CONTINUE";

  respVector = resp.CreateResponse();
  respBuffer = new char[respVector.size()]();
  GParsing::ConvertToCharPointer(respVector, respBuffer);

  SSL_write(_ssl, respBuffer, respVector.size());
  delete[] respBuffer;
}

void SendGetResponse(SSL *_ssl, const GParsing::HTTPRequest &_req) {
  GParsing::HTTPResponse resp;
  std::vector<unsigned char> respVector;
  char *respBuffer;
  std::string msg;

  msg = "Welcome to my site!";

  resp.version = "HTTP/1.1";
  resp.response_code = 200;
  resp.response_code_message = "OK";
  resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Connection", {"close"}));
  resp.message = GParsing::ConvertToCharArray(msg.c_str(), msg.length());

  respVector.clear();
  respVector = resp.CreateResponse();

  respBuffer = new char[respVector.size()];

  GParsing::ConvertToCharPointer(respVector, respBuffer);
  SSL_write(_ssl, respBuffer, respVector.size());

  delete[] respBuffer;
}
} // namespace AdvancedWebserver
