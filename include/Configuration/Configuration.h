#pragma once
#include "Configuration/ConfigurationType.h"
#include <filesystem>
#include <string>

namespace AdvancedWebserver {

class Configuration {
public:
  Configuration(
      const std::string &_uri = "/",
      const AdvancedWebserver::ConfigurationType &_type = AdvancedWebserver::
          ConfigurationTypes[AdvancedWebserver::Types::FILE_IO],
      const std::string &_file = "", const std::string &_fileType = "");
  Configuration(Configuration &&) = default;
  Configuration(const Configuration &) = default;
  Configuration &operator=(Configuration &&) = default;
  Configuration &operator=(const Configuration &) = default;
  ~Configuration();

  bool WriteFile(const std::filesystem::path &_dataDir,
                 const char &_slashReplace = SLASH_REPLACE);

  bool ReadFile(const std::string &_uri, const std::filesystem::path &_dataDir,
                const char &_slashReplace = SLASH_REPLACE);

  bool ReadFile(const std::filesystem::path &_dataDir,
                const char &_slashReplace = SLASH_REPLACE);

  const std::string &GetURI() const;
  const AdvancedWebserver::ConfigurationType &GetConfigurationType() const;
  const std::filesystem::path &GetFilePath() const;
  const std::string &GetFileType() const;

  void SetURI(const std::string &_uri);
  void SetConfigurationType(const AdvancedWebserver::ConfigurationType &_ct);
  void SetFilePath(const std::filesystem::path &_file);
  void SetFileType(const std::string &_fileType);

private:
  static constexpr const char SLASH_REPLACE = '@';
  std::string m_uri;
  std::filesystem::path m_file;
  std::string m_fileType;
  AdvancedWebserver::ConfigurationType m_type;
};

} // namespace AdvancedWebserver