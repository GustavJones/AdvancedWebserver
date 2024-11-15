#include "Configuration/Configuration.h"
#include "Configuration/ConfigurationType.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace AdvancedWebserver {

Configuration::Configuration(const std::string &_uri,
                             const AdvancedWebserver::ConfigurationType &_type,
                             const std::string &_path,
                             const std::string &_fileType)
    : m_uri(_uri), m_type(_type), m_path(_path), m_fileType(_fileType) {}

bool Configuration::WriteFile(const std::filesystem::path &_dataDir,
                              const char &_slashReplace) {
  try {
    std::string uri = GetURI();
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
    output += std::filesystem::absolute(m_path).string() + ':';
    output += m_fileType;

    if (uri[uri.length() - 1] != _slashReplace) {
      if (m_type.GetType() == AdvancedWebserver::ConfigurationTypes
                                  [AdvancedWebserver::Types::FOLDER_IO]
                                      .GetType() ||
          m_type.GetType() ==
              AdvancedWebserver::ConfigurationTypes
                  [AdvancedWebserver::Types::CASCADING_EXECUTABLE]
                      .GetType()) {
        uri += _slashReplace;
      }
    } else {
      if (m_type.GetType() == AdvancedWebserver::ConfigurationTypes
                                  [AdvancedWebserver::Types::FILE_IO]
                                      .GetType() ||
          m_type.GetType() == AdvancedWebserver::ConfigurationTypes
                                  [AdvancedWebserver::Types::EXECUTABLE]
                                      .GetType()) {
        uri.erase(uri.length() - 1);
      }
    }

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
  std::fstream f;
  std::string f_content;
  char *f_raw_content;
  int f_length;
  std::filesystem::path path;

  int delimiterIndexOld;
  int delimiterIndex;
  std::string type;
  std::string filename;

  if (IsStaticType(_dataDir, _slashReplace)) {
    path = GetStaticPath(_dataDir, _slashReplace);
    filename = "";
  } else if (IsDynamicType(_dataDir, _slashReplace)) {
    path = GetDynamicPath(_dataDir, _slashReplace);
    filename = GetDynamicFilename(GetURI(), _slashReplace);
  } else {
    return false;
  }

  // Get info from file at path
  f.open(path, std::ios::in | std::ios::ate);
  f_length = f.tellg();
  f.seekg(f.beg);

  f_raw_content = new char[f_length]();

  f.read(f_raw_content, f_length);
  for (int i = 0; i < f_length; i++) {
    f_content += f_raw_content[i];
  }

  delete[] f_raw_content;
  f.close();

  // Type
  delimiterIndex = f_content.find(':');
  if (delimiterIndex == f_content.npos) {
    return false;
  }

  type = f_content.substr(0, delimiterIndex);
  for (const auto &conf : AdvancedWebserver::ConfigurationTypes) {
    if (type == conf.GetType()) {
      SetConfigurationType(conf);
      break;
    }
  }

  // Path
  delimiterIndexOld = delimiterIndex;
  delimiterIndex = f_content.find(':', delimiterIndexOld + 1);
  if (delimiterIndex == f_content.npos) {
    return false;
  }

  SetPath(f_content.substr(delimiterIndexOld + 1,
                           delimiterIndex - delimiterIndexOld - 1));

  if (filename != "") {
    SetPath(GetPath() / filename);
  }

  // File type
  SetFileType(f_content.substr(delimiterIndex + 1));

  return true;
}

bool Configuration::ReadFile(const std::string &_uri,
                             const std::filesystem::path &_dataDir,
                             const char &_slashReplace) {
  SetURI(_uri);
  return ReadFile(_dataDir, _slashReplace);
}

const std::string &Configuration::GetURI() const { return m_uri; }
const AdvancedWebserver::ConfigurationType &
Configuration::GetConfigurationType() const {
  return m_type;
}
const std::filesystem::path &Configuration::GetPath() const { return m_path; }
const std::string &Configuration::GetFileType() const { return m_fileType; }

void Configuration::SetURI(const std::string &_uri) { m_uri = _uri; }
void Configuration::SetConfigurationType(
    const AdvancedWebserver::ConfigurationType &_ct) {
  m_type = _ct;
}
void Configuration::SetPath(const std::filesystem::path &_path) {
  m_path = _path;
}
void Configuration::SetFileType(const std::string &_fileType) {
  m_fileType = _fileType;
}

Configuration::~Configuration() {}

std::filesystem::path
Configuration::GetStaticPath(const std::filesystem::path &_dataDir,
                             const char &_slashReplace) {
  std::filesystem::path path =
      _dataDir / ConvertSlashes(GetURI(), _slashReplace);
  return path;
}

bool Configuration::IsStaticType(const std::filesystem::path &_dataDir,
                                 const char &_slashReplace) {
  std::filesystem::path path;
  std::fstream f;
  char *f_raw_content;
  std::string f_content;
  int f_length;

  int delimiterIndex;
  std::string type;

  path = GetStaticPath(_dataDir, _slashReplace);
  if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
    // Check if FILE_IO or EXECUTABLE type
    f.open(path, std::ios::in | std::ios::ate);
    f_length = f.tellg();
    f.seekg(f.beg);

    f_raw_content = new char[f_length]();

    f.read(f_raw_content, f_length);
    for (int i = 0; i < f_length; i++) {
      f_content += f_raw_content[i];
    }

    delete[] f_raw_content;
    f.close();

    delimiterIndex = f_content.find(':');

    if (delimiterIndex == f_content.npos) {
      return false;
    }

    type = f_content.substr(0, delimiterIndex);

    if (type == AdvancedWebserver::ConfigurationTypes
                    [AdvancedWebserver::Types::FILE_IO]
                        .GetType() ||
        type == AdvancedWebserver::ConfigurationTypes
                    [AdvancedWebserver::Types::EXECUTABLE]
                        .GetType()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

std::filesystem::path
Configuration::GetDynamicPath(const std::filesystem::path &_dataDir,
                              const char &_slashReplace) {
  int index = 0;
  int last_index = 0;
  std::string filename;
  std::string filename_reversed;
  std::filesystem::path path =
      _dataDir / ConvertSlashes(GetURI(), _slashReplace);
  std::string path_str = path.string();

  filename = GetDynamicFilename(GetURI(), _slashReplace);

  while (index != path_str.npos) {
    index = path_str.find(filename, last_index);
    if (index != path_str.npos) {
      last_index = index + 1;
    }
  }

  last_index--;

  path_str.erase(last_index);
  path = path_str;

  return path;
}

bool Configuration::IsDynamicType(const std::filesystem::path &_dataDir,
                                  const char &_slashReplace) {
  std::filesystem::path path;
  std::fstream f;
  char *f_raw_content;
  std::string f_content;
  int f_length;

  int delimiterIndex;
  std::string type;

  path = GetDynamicPath(_dataDir, _slashReplace);
  if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
    // Check if FOLDER_IO or CASCADING_EXECUTABLE type
    f.open(path, std::ios::in | std::ios::ate);
    f_length = f.tellg();
    f.seekg(f.beg);

    f_raw_content = new char[f_length]();

    f.read(f_raw_content, f_length);
    for (int i = 0; i < f_length; i++) {
      f_content += f_raw_content[i];
    }

    delete[] f_raw_content;
    f.close();

    delimiterIndex = f_content.find(':');

    if (delimiterIndex == f_content.npos) {
      return false;
    }

    type = f_content.substr(0, delimiterIndex);

    if (type == AdvancedWebserver::ConfigurationTypes
                    [AdvancedWebserver::Types::FOLDER_IO]
                        .GetType() ||
        type == AdvancedWebserver::ConfigurationTypes
                    [AdvancedWebserver::Types::CASCADING_EXECUTABLE]
                        .GetType()) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

std::string Configuration::ConvertSlashes(const std::string &_uri,
                                          const char &_slashReplace) {
  std::string uri = _uri;
  for (char &c : uri) {
    if (c == '/') {
      c = _slashReplace;
    }
  }

  return uri;
}

bool Configuration::IsFolderConfiguration(const std::filesystem::path &_path) {
  std::fstream f;
  std::string typeStr;
  std::string parse;
  int delimiterPos;

  int fileContentLen;
  char *fileContent;

  f.open(_path, std::ios::in | std::ios::ate);
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

  if (typeStr !=
      AdvancedWebserver::ConfigurationTypes[AdvancedWebserver::Types::FOLDER_IO]
          .GetType()) {
    return false;
  } else {
    return true;
  }
}

std::string Configuration::GetDynamicFilename(const std::string &_uri,
                                              const char &_slashReplace) {
  std::string filename;
  std::string filename_reversed;
  std::string path_str = ConvertSlashes(_uri, _slashReplace);

  // Last character is slash
  if (path_str[path_str.length() - 1] == _slashReplace &&
      path_str.length() != 1) {
    for (int i = path_str.length() - 2; i >= 0; i--) {
      if (path_str[i] == _slashReplace) {
        break;
      }
      filename_reversed += path_str[i];
    }
  } else if (path_str.length() != 1) {
    for (int i = path_str.length() - 1; i >= 0; i--) {
      if (path_str[i] == _slashReplace) {
        break;
      }
      filename_reversed += path_str[i];
    }
  } else {
    filename_reversed = "";
  }

  // Reverse filename
  for (int i = filename_reversed.length() - 1; i >= 0; i--) {
    filename += filename_reversed[i];
  }

  return filename;
}
} // namespace AdvancedWebserver
