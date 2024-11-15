#include "Server/HandleConnection.h"
#include "Configuration/Configuration.h"
#include "Configuration/ConfigurationType.h"
#include "Core/Core.h"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/types.h>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace AdvancedWebserver {

void HandleConnection(SSL_CTX *_sslContext, GNetworking::Socket _clientSock,
                      const std::filesystem::path &_dataDir) {
  AdvancedWebserver::DATA_DIR = _dataDir;

  bool handling_requests = true;
  SSL *ssl;

  ssl = SSL_new(_sslContext);
  SSL_set_fd(ssl, _clientSock.sock);

  struct pollfd pfd;
  pfd.fd = _clientSock.sock;
  pfd.events = POLLRDHUP;

  if (poll(&pfd, 1, 0) == 0) {
    SSL_accept(ssl);

    while (handling_requests) {
      handling_requests = HandleRequest(ssl);
    }
  } else {
    std::cout << "----------------" << std::endl;
    LOG("Socket Closed before SSL Handshake");
    std::cout << "----------------" << std::endl;
  }

  int sock_close = SSL_get_fd(ssl);
  SSL_shutdown(ssl);
  SSL_free(ssl);

  close(sock_close);
}

bool HandleRequest(SSL *_ssl) {
  constexpr const bool ContinueOverride = true;

  GParsing::HTTPRequest req;
  constexpr const int BUFFER_LENGTH = 1024 * 8;
  int bufferReadLen;
  bool hasHostHeader;

  char *buffer = new char[BUFFER_LENGTH]();
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    buffer[i] = 0;
  }

  int retval;
  struct pollfd pfd;
  pfd.fd = SSL_get_fd(_ssl);
  pfd.events = POLLRDHUP;
  retval = poll(&pfd, 1, 0);

  if (retval < 0 || retval > 0) {
    LOG("Client disconnected");
    return false;
  }

  // Accept Requests
  bufferReadLen = SSL_read(_ssl, buffer, BUFFER_LENGTH);

  // Break if connection ends
  if (bufferReadLen <= 0) {
    delete[] buffer;
    return false;
  }

  // Parse Request
  try {
    req.ParseRequest(GParsing::ConvertToCharArray(buffer, bufferReadLen));
    delete[] buffer;
  } catch (const std::exception &e) {
    std::cerr << "GParsing error" << ": " << e.what() << std::endl;
    delete[] buffer;
    return false;
  }

  if (!HasHostHeader(req) && req.version == "HTTP/1.1") {
    SendHostHeaderErrorResponse(_ssl);
    return false;
  }

  if (HasHeaderWithValue(req, "Expect", "100-continue")) {
    SendContinueResponse(_ssl);
  } else if (ContinueOverride) {
    SendContinueResponse(_ssl);
  }

  // Handle Requests
  switch (req.method) {
  case GParsing::HTTPMethod::GET: {
    bool success = SendGetResponse(_ssl, req);
    LOG("Sent response for GET request: " + req.uri);

    if (success) {
      if (HasHeaderWithValue(req, "Connection", "close")) {
        success = false;
      }
    }

    return success;
    break;
  }

  default:
    break;
  }

  return false;
}

bool HasHeaderWithValue(const GParsing::HTTPRequest &_req,
                        const std::string &_header, const std::string &_value) {
  bool hasHeaderKey = false;
  bool hasHeaderValue = false;

  for (const auto &header : _req.headers) {
    if (header.first == _header) {
      hasHeaderKey = true;

      if (_value != "") {
        for (const auto &value : header.second) {
          if (value == _value) {
            hasHeaderValue = true;
          }
        }
      } else {
        hasHeaderValue = true;
      }
    }
  }

  return hasHeaderKey && hasHeaderValue;
}

bool HasHeaderWithValue(const GParsing::HTTPResponse &_req,
                        const std::string &_header, const std::string &_value) {
  bool hasHeaderKey = false;
  bool hasHeaderValue = false;

  for (const auto &header : _req.headers) {
    if (header.first == _header) {
      hasHeaderKey = true;

      if (_value != "") {
        for (const auto &value : header.second) {
          if (value == _value) {
            hasHeaderValue = true;
          }
        }
      } else {
        hasHeaderValue = true;
      }
    }
  }

  return hasHeaderKey && hasHeaderValue;
}

bool HasHostHeader(const GParsing::HTTPRequest &_req) {
  return HasHeaderWithValue(_req, "Host");
}

void SendHostHeaderErrorResponse(SSL *_ssl) {
  GParsing::HTTPResponse resp;
  std::vector<unsigned char> respVector;
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
  resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Connection", {"close"}));

  resp.message = GParsing::ConvertToCharArray(msg.c_str(), msg.length());

  respVector = resp.CreateResponse();
  SendBuffer(_ssl, respVector);
}

void SendContinueResponse(SSL *_ssl) {
  GParsing::HTTPResponse resp;
  std::vector<unsigned char> respVector;

  resp.version = "HTTP/1.1";
  resp.response_code = 100;
  resp.response_code_message = "CONTINUE";

  respVector = resp.CreateResponse();
  SendBuffer(_ssl, respVector);
}

bool SendGetResponse(SSL *_ssl, const GParsing::HTTPRequest &_req) {
  // Fix connections not closing
  constexpr const bool CloseConnectionOnSuccess = false;

  AdvancedWebserver::Configuration c;

  // Load configuration from disk according to URI
  if (!LoadConfiguration(c, _req.uri, _ssl)) {
    return false;
  }

  // For different configurations
  if (c.GetConfigurationType().GetType() ==
      AdvancedWebserver::ConfigurationTypes[AdvancedWebserver::FILE_IO]
          .GetType()) {
    return SendGetFileIOResponse(c, _ssl, CloseConnectionOnSuccess);
  }

  else if (c.GetConfigurationType().GetType() ==
           AdvancedWebserver::ConfigurationTypes[AdvancedWebserver::FOLDER_IO]
               .GetType()) {
    return SendGetFolderIOResponse(c, _ssl, CloseConnectionOnSuccess);

  } else if (c.GetConfigurationType().GetType() ==
             AdvancedWebserver::ConfigurationTypes
                 [AdvancedWebserver::EXECUTABLE]
                     .GetType()) {
    return SendGetExecutableResponse(c, _ssl, CloseConnectionOnSuccess);
  } else if (c.GetConfigurationType().GetType() ==
             AdvancedWebserver::ConfigurationTypes
                 [AdvancedWebserver::CASCADING_EXECUTABLE]
                     .GetType()) {
    return SendGetCascadingExecutableResponse(c, _ssl,
                                              CloseConnectionOnSuccess);
  }
  return false;
}

int SendBuffer(SSL *_ssl, std::vector<unsigned char> _buff) {
  int sent;
  char *outBuffer;
  outBuffer = new char[_buff.size()]();
  GParsing::ConvertToCharPointer(_buff, outBuffer);

  int retval;
  struct pollfd pfd;
  pfd.fd = SSL_get_fd(_ssl);
  pfd.events = POLLRDHUP;
  retval = poll(&pfd, 1, 0);

  if (retval != 0) {
    delete[] outBuffer;
    return -1;
  }

  try {
    sent = SSL_write(_ssl, outBuffer, _buff.size());
  } catch (const std::exception &) {
    sent = -1;
  }
  delete[] outBuffer;
  return sent;
}

std::string GetCurrentDate() {
  std::string out;

  const int DATE_LEN = 64;
  char *dateStr = new char[DATE_LEN];

  for (int i = 0; i < DATE_LEN; i++) {
    dateStr[i] = 0;
  }

  const auto now = std::chrono::system_clock::now();
  time_t tnow = std::chrono::system_clock::to_time_t(now);

  std::strftime(dateStr, DATE_LEN, "%a, %d %b %Y %H:%M:%S GMT",
                std::localtime(&tnow));

  for (int i = 0; i < DATE_LEN; i++) {
    if (dateStr[i] != 0) {
      out += dateStr[i];
    } else {
      break;
    }
  }

  delete[] dateStr;
  return out;
}

std::time_t ParseDate(const std::string &_time) {
  struct std::tm tm;
  time_t t;
  strptime(_time.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tm);
  t = mktime(&tm);

  return t;
}

bool LoadConfiguration(AdvancedWebserver::Configuration &_c,
                       const std::string &_uri, SSL *_ssl) {
  std::filesystem::path filePath;
  GParsing::HTTPResponse _resp;
  std::vector<unsigned char> _resp_vector;

  if (!_c.ReadFile(_uri, AdvancedWebserver::DATA_DIR)) {
    // Configuration doesn't exist, unknown URI
    LOG("URI configuration " + _c.GetURI() + " cannot be found");
    _resp.version = "HTTP/1.1";
    _resp.response_code = 404;
    _resp.response_code_message = "Not Found";
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"close"}));
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);

    return false;
  }

  if (_c.GetFilename() != "") {
    filePath = _c.GetPath() / _c.GetFilename();
  } else {
    filePath = _c.GetPath();
  }

  if (!std::filesystem::exists(filePath)) {
    LOG("File cannot be found found in configuration " + _c.GetURI());
    _resp.version = "HTTP/1.1";
    _resp.response_code = 404;
    _resp.response_code_message = "Not Found";
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"close"}));
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);

    return false;
  }

  return true;
}

bool SendGetFileIOResponse(Configuration &_c, SSL *_ssl,
                           bool _closeConnectionsOnSuccess) {
  GParsing::HTTPResponse _resp;
  std::vector<unsigned char> _resp_vector;

  std::filesystem::path filePath;
  std::fstream file;
  int fileSize;
  char *buf;

  if (_c.GetFilename() != "") {
    filePath = _c.GetPath() / _c.GetFilename();
  } else {
    filePath = _c.GetPath();
  }

  // File_IO Configuration
  _resp.response_code = 200;
  _resp.version = "HTTP/1.1";
  _resp.response_code_message = "OK";

  file.open(filePath, std::ios::in | std::ios::ate);
  fileSize = file.tellg();
  file.seekg(file.beg);

  buf = new char[fileSize]();

  file.read(buf, fileSize);
  file.close();

  std::vector<unsigned char> messageVector =
      GParsing::ConvertToCharArray(buf, fileSize);
  delete[] buf;

  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Content-Type", {_c.GetFileType()}));

  _resp.message = messageVector;
  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Content-Length", {std::to_string(_resp.message.size())}));

  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Date", {GetCurrentDate()}));

  if (_closeConnectionsOnSuccess) {
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"close"}));
  } else {
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"keep-alive"}));
  }

  _resp_vector = _resp.CreateResponse();
  SendBuffer(_ssl, _resp_vector);
  if (_closeConnectionsOnSuccess) {
    return false;
  } else {
    return true;
  }
}

bool SendGetFolderIOResponse(AdvancedWebserver::Configuration &_c, SSL *_ssl,
                             bool _closeConnectionsOnSuccess) {
  GParsing::HTTPResponse _resp;
  std::vector<unsigned char> _resp_vector;

  std::filesystem::path filePath;
  std::fstream file;
  int fileSize;
  char *buf;

  if (_c.GetFilename() != "") {
    filePath = _c.GetPath() / _c.GetFilename();
  } else {
    filePath = _c.GetPath();
  }

  // Folder_IO Configuration
  if (!std::filesystem::exists(filePath) ||
      !std::filesystem::is_regular_file(filePath)) {
    LOG("Path " + (filePath).string() + " doesn't exist cannot GET");

    _resp.version = "HTTP/1.1";
    _resp.response_code = 404;
    _resp.response_code_message = "Not Found";
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"close"}));
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);

    return false;
  }

  file.open(filePath, std::ios::in | std::ios::ate);
  fileSize = file.tellg();
  file.seekg(file.beg);

  buf = new char[fileSize]();

  file.read(buf, fileSize);
  file.close();

  std::vector<unsigned char> messageVector =
      GParsing::ConvertToCharArray(buf, fileSize);
  delete[] buf;

  _resp.version = "HTTP/1.1";
  _resp.response_code = 200;
  _resp.response_code_message = "OK";

  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Content-Type", {_c.GetFileType()}));

  _resp.message = messageVector;
  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Content-Length", {std::to_string(_resp.message.size())}));

  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Date", {GetCurrentDate()}));

  if (_closeConnectionsOnSuccess) {
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"close"}));
  } else {
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"keep-alive"}));
  }

  SendBuffer(_ssl, _resp.CreateResponse());

  if (_closeConnectionsOnSuccess) {
    return false;
  } else {
    return true;
  }
}

bool SendGetExecutableResponse(AdvancedWebserver::Configuration &_c, SSL *_ssl,
                               bool _closeConnectionsOnSuccess) {
  GParsing::HTTPResponse _resp;
  std::vector<unsigned char> _resp_vector;

  // Executable Configuration
  // TODO
  _resp.version = "HTTP/1.1";
  _resp.response_code = 501;
  _resp.response_code_message = "Not Implemented";
  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Connection", {"close"}));
  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Date", {GetCurrentDate()}));

  _resp_vector = _resp.CreateResponse();
  SendBuffer(_ssl, _resp_vector);
  return false;
}

bool SendGetCascadingExecutableResponse(AdvancedWebserver::Configuration &_c,
                                        SSL *_ssl,
                                        bool _closeConnectionsOnSuccess) {
  GParsing::HTTPResponse _resp;
  std::vector<unsigned char> _resp_vector;

  // Cascading Executable Configuration
  // TODO
  _resp.version = "HTTP/1.1";
  _resp.response_code = 501;
  _resp.response_code_message = "Not Implemented";
  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Connection", {"close"}));
  _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Date", {GetCurrentDate()}));

  _resp_vector = _resp.CreateResponse();
  SendBuffer(_ssl, _resp_vector);
  return false;
}

} // namespace AdvancedWebserver
