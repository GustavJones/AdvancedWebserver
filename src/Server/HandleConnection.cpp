#include "Server/HandleConnection.hpp"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include "GParsing/HTTP/HTTPMethod.hpp"
#include "GParsing/HTTP/HTTPRequest.hpp"
#include "GParsing/HTTP/HTTPResponse.hpp"
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/types.h>
#include <utility>
#include <vector>

namespace AdvancedWebserver {

void HandleConnection(SSL_CTX *_sslContext, GNetworking::Socket _clientSock,
                      bool *active) {
  constexpr const int BUFFER_LENGTH = 2048;
  char *response_buffer;
  char *buffer = new char[BUFFER_LENGTH]();
  bool handling_requests = true;
  SSL *ssl;

  GParsing::HTTPRequest req;
  GParsing::HTTPResponse resp;

  for (int i = 0; i < BUFFER_LENGTH; i++) {
    buffer[i] = 0;
  }

  ssl = SSL_new(_sslContext);
  SSL_set_fd(ssl, _clientSock.sock);
  SSL_accept(ssl);

  while (handling_requests) {
    // Accept Requests
    SSL_read(ssl, buffer, BUFFER_LENGTH);

    // Break if connection ends
    if (buffer[0] == 0) {
      break;
    }

    // Parse Request
    try {
      req.ParseRequest(GParsing::ConvertToCharArray(buffer, BUFFER_LENGTH));

    } catch (const std::exception &e) {
      std::cerr << "GParsing error" << ": " << e.what() << std::endl;
      handling_requests = false;
    }

    switch (req.method) {
    case GParsing::HTTPMethod::GET: {
      resp.version = "HTTP/1.1";
      resp.response_code = 100;
      resp.response_code_message = "CONTINUE";

      std::vector<unsigned char> respVector = resp.CreateResponse();
      response_buffer = new char[respVector.size()]();
      GParsing::ConvertToCharPointer(respVector, response_buffer);

      SSL_write(ssl, response_buffer, respVector.size());
      delete[] response_buffer;

      std::string msg = "Welcome to my site!";
      std::pair<std::string, std::vector<std::string>> header;
      header.first = "Connection";
      header.second.push_back("close");
      resp.version = "HTTP/1.1";
      resp.response_code = 200;
      resp.response_code_message = "OK";
      resp.headers.push_back(header);
      resp.message = GParsing::ConvertToCharArray(msg.c_str(), msg.length());

      respVector.clear();
      respVector = resp.CreateResponse();

      response_buffer = new char[respVector.size()];

      GParsing::ConvertToCharPointer(respVector, response_buffer);
      SSL_write(ssl, response_buffer, respVector.size());

      delete[] response_buffer;
      handling_requests = false;
      break;
    }

    default:
      break;
    }
  }

  delete[] buffer;
  SSL_shutdown(ssl);
  SSL_free(ssl);

  *active = false;
}
} // namespace AdvancedWebserver
