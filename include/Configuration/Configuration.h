#pragma once
#include "Configuration/ConfigurationType.h"
#include <filesystem>
#include <string>
#include "Core.h"

namespace AdvancedWebserver {

class Configuration {
public:
  ADVANCEDWEBSERVER_CONFIGURATION_API Configuration(
      const std::string &_uri = "/",
      const AdvancedWebserver::ConfigurationType &_type = AdvancedWebserver::
          ConfigurationTypes[AdvancedWebserver::Types::FILE_IO],
      const std::string &_path = "", const std::string &_fileType = "");
  ADVANCEDWEBSERVER_CONFIGURATION_API Configuration(Configuration &&) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API Configuration(const Configuration &) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API Configuration &operator=(Configuration &&) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API Configuration &operator=(const Configuration &) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API ~Configuration();

  ADVANCEDWEBSERVER_CONFIGURATION_API bool WriteFile(const std::filesystem::path &_dataDir,
                             const char &_slashReplace = SLASH_REPLACE);

  ADVANCEDWEBSERVER_CONFIGURATION_API bool ReadFile(const std::string &_uri,
                            const std::filesystem::path &_dataDir,
                            const char &_slashReplace = SLASH_REPLACE);

  ADVANCEDWEBSERVER_CONFIGURATION_API bool ReadFile(const std::filesystem::path &_dataDir,
                            const char &_slashReplace = SLASH_REPLACE);

  ADVANCEDWEBSERVER_CONFIGURATION_API const std::string &GetURI() const;
  ADVANCEDWEBSERVER_CONFIGURATION_API const AdvancedWebserver::ConfigurationType &
  GetConfigurationType() const;
  ADVANCEDWEBSERVER_CONFIGURATION_API const std::filesystem::path &GetPath() const;
  ADVANCEDWEBSERVER_CONFIGURATION_API const std::string &GetFileType() const;

  ADVANCEDWEBSERVER_CONFIGURATION_API void SetURI(const std::string &_uri);
  ADVANCEDWEBSERVER_CONFIGURATION_API void
  SetConfigurationType(const AdvancedWebserver::ConfigurationType &_ct);
  ADVANCEDWEBSERVER_CONFIGURATION_API void SetPath(const std::filesystem::path &_path);
  ADVANCEDWEBSERVER_CONFIGURATION_API void SetFileType(const std::string &_fileType);

private:
  static constexpr const char SLASH_REPLACE = '@';
  std::string m_uri;
  std::filesystem::path m_path;
  std::string m_fileType;
  AdvancedWebserver::ConfigurationType m_type;

  std::filesystem::path GetStaticPath(const std::filesystem::path &_dataDir,
                                      const char &_slashReplace);
  bool IsStaticType(const std::filesystem::path &_dataDir,
                    const char &_slashReplace);

  std::filesystem::path GetDynamicPath(const std::filesystem::path &_dataDir,
                                       const char &_slashReplace);
  bool IsDynamicType(const std::filesystem::path &_dataDir,
                     const char &_slashReplace);

  std::string ConvertSlashes(const std::string &_uri,
                             const char &_slashReplace);

  std::string GetDynamicFilename(const std::string &_uri,
                                 const char &_slashReplace);
};

} // namespace AdvancedWebserver
