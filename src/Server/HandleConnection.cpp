#include "Server/HandleConnection.h"
#include "Configuration/Configuration.h"
#include "Configuration/ConfigurationType.h"
#include "Core/Core.h"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include "GParsing/HTTP/HTTPMethod.hpp"
#include "GParsing/HTTP/HTTPRequest.hpp"
#include "GParsing/HTTP/HTTPResponse.hpp"
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/types.h>
#include <string>
#include <utility>
#include <vector>

#define LOG(x) std::cout << '[' << GetCurrentDate() << "]: " << x << std::endl

namespace AdvancedWebserver {

void HandleConnection(SSL_CTX *_sslContext, GNetworking::Socket _clientSock,
                      bool *active, const std::filesystem::path &_dataDir) {
  AdvancedWebserver::DATA_DIR = _dataDir;

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
  constexpr const int BUFFER_LENGTH = 1024 * 8;
  int bufferReadLen;
  bool hasHostHeader;

  char *buffer = new char[BUFFER_LENGTH]();
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    buffer[i] = 0;
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

  hasHostHeader = HasHostHeader(req);

  if (!hasHostHeader && req.version == "HTTP/1.1") {
    SendHostHeaderErrorResponse(_ssl);
    return false;
  }

  SendContinueResponse(_ssl);

  // Handle Requests
  switch (req.method) {
  case GParsing::HTTPMethod::GET: {
    bool success = SendGetResponse(_ssl, req);

    if (success) {
      LOG("Sent response for GET request: " + req.uri);
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
  resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
      "Connection", {"close"}));

  resp.message = GParsing::ConvertToCharArray(msg.c_str(), msg.length());

  respVector = resp.CreateResponse();
  respBuffer = new char[respVector.size()]();

  GParsing::ConvertToCharPointer(respVector, respBuffer);

  SSL_write(_ssl, respBuffer, respVector.size());
  delete[] respBuffer;
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
  GParsing::HTTPResponse _resp;
  std::vector<unsigned char> _resp_vector;
  Configuration c;

  if (!c.ReadFile(_req.uri, AdvancedWebserver::DATA_DIR)) {
    // Configuration doesn't exist, unknown URI
    LOG("URI configuration " + c.GetURI() + " cannot be found");
    _resp.version = "HTTP/1.1";
    _resp.response_code = 404;
    _resp.response_code_message = "Not Found";
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"Close"}));
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);

    return false;
  } else if (!std::filesystem::exists(c.GetFilePath())) {
    LOG("File cannot be found found in configuration " + c.GetURI());
    _resp.version = "HTTP/1.1";
    _resp.response_code = 404;
    _resp.response_code_message = "Not Found";
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"Close"}));
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);

    return false;
  }

  // For different configurations
  if (c.GetConfigurationType().GetType() ==
      AdvancedWebserver::ConfigurationTypes[AdvancedWebserver::FILE_IO]
          .GetType()) {
    // File_IO Configuration
    _resp.response_code = 200;
    _resp.version = "HTTP/1.1";
    _resp.response_code_message = "OK";

    std::fstream file;
    int fileSize;
    char *buf;

    file.open(c.GetFilePath(), std::ios::in | std::ios::ate);
    fileSize = file.tellg();
    file.seekg(file.beg);

    buf = new char[fileSize]();

    file.read(buf, fileSize);
    file.close();

    std::vector<unsigned char> messageVector =
        GParsing::ConvertToCharArray(buf, fileSize);
    delete[] buf;

    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Content-Type", {c.GetFileType()}));

    _resp.message = messageVector;
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Content-Length", {std::to_string(_resp.message.size())}));

    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);
  } else if (c.GetConfigurationType().GetType() ==
             AdvancedWebserver::ConfigurationTypes[AdvancedWebserver::FOLDER_IO]
                 .GetType()) {
    // Folder_IO Configuration
    std::filesystem::path filename;

    for (int i = c.GetURI().length() - 1; i >= 0; i--) {
      if (c.GetURI()[i] == '/') {
        filename = c.GetURI().substr(i + 1);
        break;
      }
    }

    if (!std::filesystem::exists(c.GetFilePath() / filename)) {
      LOG("Path " + (c.GetFilePath() / filename).string() +
          " doesn't exist cannot GET");

      _resp.version = "HTTP/1.1";
      _resp.response_code = 404;
      _resp.response_code_message = "Not Found";
      _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
          "Connection", {"Close"}));
      _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
          "Date", {GetCurrentDate()}));

      _resp_vector = _resp.CreateResponse();
      SendBuffer(_ssl, _resp_vector);

      return false;
    }

    std::fstream file;
    int fileSize;
    char *buf;

    file.open(c.GetFilePath() / filename, std::ios::in | std::ios::ate);
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
        "Content-Type", {c.GetFileType()}));

    _resp.message = messageVector;
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Content-Length", {std::to_string(_resp.message.size())}));

    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    if (SendBuffer(_ssl, _resp.CreateResponse()) < 0) {
      return false;
    }
  } else if (c.GetConfigurationType().GetType() ==
             AdvancedWebserver::ConfigurationTypes
                 [AdvancedWebserver::EXECUTABLE]
                     .GetType()) {
    // Executable Configuration
    // TODO
    _resp.version = "HTTP/1.1";
    _resp.response_code = 501;
    _resp.response_code_message = "Not Implemented";
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"Close"}));
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);
    return false;
  } else if (c.GetConfigurationType().GetType() ==
             AdvancedWebserver::ConfigurationTypes
                 [AdvancedWebserver::CASCADING_EXECUTABLE]
                     .GetType()) {
    // Cascading Executable Configuration
    // TODO
    _resp.version = "HTTP/1.1";
    _resp.response_code = 501;
    _resp.response_code_message = "Not Implemented";
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Connection", {"Close"}));
    _resp.headers.push_back(std::pair<std::string, std::vector<std::string>>(
        "Date", {GetCurrentDate()}));

    _resp_vector = _resp.CreateResponse();
    SendBuffer(_ssl, _resp_vector);
    return false;
  }
  return true;
}

int SendBuffer(SSL *_ssl, std::vector<unsigned char> _buff) {
  int sent;
  char *outBuffer;
  outBuffer = new char[_buff.size()]();
  GParsing::ConvertToCharPointer(_buff, outBuffer);

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

// Linux only
std::time_t ParseDate(const std::string &_time) {
  struct std::tm tm;
  time_t t;
  strptime(_time.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tm);
  t = mktime(&tm);

  return t;
}
} // namespace AdvancedWebserver
