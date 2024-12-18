#pragma once
#include <string>
#include <vector>

namespace AdvancedWebserver {
class ConfigurationType {
public:
  ConfigurationType(const std::string &_type = "",
                    const std::string &_description = "");
  ConfigurationType(ConfigurationType &&) = default;
  ConfigurationType(const ConfigurationType &) = default;
  ConfigurationType &operator=(ConfigurationType &&) = default;
  ConfigurationType &operator=(const ConfigurationType &) = default;
  ~ConfigurationType();

  // [[deprecated("Not working")]]
  // bool operator==(const ConfigurationType &_ct);

  const std::string &GetType() const;
  const std::string &GetDescription() const;

  void SetType(const std::string &_type);
  void SetDescription(const std::string &_description);

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
