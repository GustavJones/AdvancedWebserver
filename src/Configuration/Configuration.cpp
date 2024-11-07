#include "Configuration/Configuration.h"
#include "Configuration/ConfigurationType.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace AdvancedWebserver {

Configuration::Configuration(const std::string &_uri,
                             const AdvancedWebserver::ConfigurationType &_type,
                             const std::string &_file,
                             const std::string &_fileType)
    : m_uri(_uri), m_type(_type), m_file(_file), m_fileType(_fileType) {}

bool Configuration::WriteFile(const std::filesystem::path &_dataDir,
                              const char &_slashReplace) {
  try {
    std::string uri = m_uri;
    std::string path;
    std::string output;

    if (!std::filesystem::exists(_dataDir)) {
      std::filesystem::create_directory(_dataDir);
    }

    for (char &c : uri) {
      if (c == '/') {
        c = _slashReplace;
      }
    }

    output += m_type.GetType() + ':';
    output += std::filesystem::absolute(m_file).string() + ':';
    output += m_fileType;

    std::fstream f;

    path = _dataDir / uri;
    f.open(path, std::ios::out);

    f.write(output.c_str(), output.length());

    f.close();

    return true;
  } catch (const std::exception &e) {
    return false;
  }
}

bool Configuration::ReadFile(const std::filesystem::path &_dataDir,
                             const char &_slashReplace) {
  try {
    bool foundType = false;
    int delimiterPos;
    std::string typeStr;
    std::string uri = m_uri;
    std::string path;
    std::string parse;
    char *fileContent;
    int fileContentLen;

    if (!std::filesystem::exists(_dataDir)) {
      std::filesystem::create_directory(_dataDir);
    }

    for (char &c : uri) {
      if (c == '/') {
        c = _slashReplace;
      }
    }

    std::fstream f;

    path = _dataDir / uri;

    if (!std::filesystem::exists(path)) {
      std::filesystem::path uri_temp;
      for (const char &c : uri) {
        if (c != _slashReplace) {
          uri_temp += c;
        } else {
          uri_temp += '/';
        }
      }
      path = _dataDir / ('@' + uri_temp.relative_path().string());
      path.erase(path.find(uri_temp.filename()) - 1);
      if (!std::filesystem::exists(path) ||
          !std::filesystem::is_regular_file(path)) {
        throw std::runtime_error("File not found");
      }
    }

    f.open(path, std::ios::in | std::ios::ate);
    fileContentLen = f.tellg();
    f.seekg(f.beg);

    fileContent = new char[fileContentLen]();

    f.read(fileContent, fileContentLen);
    f.close();

    parse = "";
    for (int i = 0; i < fileContentLen; i++) {
      parse += fileContent[i];
    }
    delete[] fileContent;

    delimiterPos = parse.find(':');

    if (delimiterPos == parse.npos) {
      throw std::runtime_error("Cannot find a readable configuration");
    }

    typeStr = parse.substr(0, delimiterPos);

    for (const auto &type : AdvancedWebserver::ConfigurationTypes) {
      if (type.GetType() == typeStr) {
        foundType = true;
        m_type = type;
      }
    }

    if (!foundType) {
      throw std::runtime_error("Cannot find a readable configuration");
    }

    parse.erase(0, delimiterPos + 1);
    delimiterPos = parse.find(':');

    m_file = parse.substr(0, delimiterPos);

    parse.erase(0, delimiterPos + 1);
    m_fileType = parse;

    return true;
  } catch (const std::exception &e) {
    return false;
  }
}

bool Configuration::ReadFile(const std::string &_uri,
                             const std::filesystem::path &_dataDir,
                             const char &_slashReplace) {
  m_uri = _uri;
  return ReadFile(_dataDir, _slashReplace);
}

const std::string &Configuration::GetURI() const { return m_uri; }
const AdvancedWebserver::ConfigurationType &
Configuration::GetConfigurationType() const {
  return m_type;
}
const std::filesystem::path &Configuration::GetFilePath() const {
  return m_file;
}
const std::string &Configuration::GetFileType() const { return m_fileType; }

void Configuration::SetURI(const std::string &_uri) { m_uri = _uri; }
void Configuration::SetConfigurationType(
    const AdvancedWebserver::ConfigurationType &_ct) {
  m_type = _ct;
}
void Configuration::SetFilePath(const std::filesystem::path &_file) {
  m_file = _file;
}
void Configuration::SetFileType(const std::string &_fileType) {
  m_fileType = _fileType;
}

Configuration::~Configuration() {}
} // namespace AdvancedWebserver
