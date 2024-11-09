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
      const std::string &_path = "", const std::string &_fileType = "");
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
  const std::filesystem::path &GetPath() const;
  const std::string &GetFileType() const;
  const std::string &GetFilename() const;

  void SetURI(const std::string &_uri);
  void SetConfigurationType(const AdvancedWebserver::ConfigurationType &_ct);
  void SetPath(const std::filesystem::path &_path);
  void SetFileType(const std::string &_fileType);
  void SetFilename(const std::string &_filename);

private:
  static constexpr const char SLASH_REPLACE = '@';
  std::string m_filename;
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

  bool IsFolderConfiguration(const std::filesystem::path &_path);

  std::string GetDynamicFilename(const std::string &_uri,
                                 const char &_slashReplace);
};

} // namespace AdvancedWebserver
