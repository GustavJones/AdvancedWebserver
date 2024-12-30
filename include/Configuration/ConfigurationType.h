#pragma once
#include "Core.h"
#include <string>
#include <vector>

namespace AdvancedWebserver {
class ConfigurationType {
public:
  ADVANCEDWEBSERVER_CONFIGURATION_API ConfigurationType(const std::string &_type = "",
                    const std::string &_description = "");
  ADVANCEDWEBSERVER_CONFIGURATION_API ConfigurationType(ConfigurationType &&) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API ConfigurationType(const ConfigurationType &) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API ConfigurationType &operator=(ConfigurationType &&) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API ConfigurationType &operator=(const ConfigurationType &) = default;
  ADVANCEDWEBSERVER_CONFIGURATION_API ~ConfigurationType();

  // [[deprecated("Not working")]]
  // bool operator==(const ConfigurationType &_ct);

  ADVANCEDWEBSERVER_CONFIGURATION_API const std::string &GetType() const;
  ADVANCEDWEBSERVER_CONFIGURATION_API const std::string &GetDescription() const;

  ADVANCEDWEBSERVER_CONFIGURATION_API void SetType(const std::string &_type);
  ADVANCEDWEBSERVER_CONFIGURATION_API void SetDescription(const std::string &_description);

private:
  std::string m_type;
  std::string m_description;
};

// Pre defined types of configurations
static const std::vector<ConfigurationType> ConfigurationTypes = {
    {"file_io", "Read/Write a file from disk and return content. Works for a "
                "static URI path and a single file."},
    {"folder_io",
     "Read/Write a file from disk and return content. Works with "
     "dynamic URI path and multiple corresponding files in the folder."},
    {"executable",
     "Execute a file with a path to the request in a temp file as an argument "
     "and response as output to the same temp file. Works for a static URI "
     "path and a single executable."},
    {"cascading_executable",
     "Execute a file with a path to the request in a temp file as an "
     "argument and response as output to the same temp file. Works for a "
     "dynamic URI path and a single executable."}};

enum Types { FILE_IO, FOLDER_IO, EXECUTABLE, CASCADING_EXECUTABLE };

} // namespace AdvancedWebserver
