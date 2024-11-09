#include "Configuration/Configuration.h"
#include "Configuration/ConfigurationType.h"
#include "Core/Core.h"
#include "GArgs/GArgs.hpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

int main(int argc, char *argv[]) {
  bool validConfigurationType = false;

  GArgs::Parser p("AdvancedWebserver Configure Tool", "V1.0", true);
  p.AddStructure(
      "[flags:help=Application "
      "flags,argument_filter=-,value_amount=0;uri:help=The uri the webserver "
      "will access;type:help=Set the type of action to perform on URI "
      "request;file:help=The file or program to access from "
      "URI;http_file_type:help=Set the Content-Type of the file]");
  p.AddKey(GArgs::Key("flags", "--help | -h", "Display this message"));
  p.AddKey(GArgs::Key("flags", "--set-data-dir",
                      "Set the directory where the Webserver data is stored"));
  p.AddKey(GArgs::Key("uri", "*", "URI address/addresses to configure"));
  p.AddKey(GArgs::Key("file", "*", "File/Folder or program path to configure"));
  p.AddKey(GArgs::Key("http_file_type", "text/html",
                      "Set the type to html document"));

  for (const auto &type : AdvancedWebserver::ConfigurationTypes) {
    p.AddKey(GArgs::Key("type", type.GetType(), type.GetDescription()));
  }

  p.ParseArgs(argc, argv);

  if (p.Contains("flags", "--help") || p.Contains("flags", "-h")) {
    p.DisplayHelp();
    return 0;
  }

  if (p.Contains("flags", "--set-data-dir=")) {
    for (const auto &flag : p["flags"]) {
      if (flag.find("--set-data-dir=") != flag.npos) {
        std::string relativePath = flag.substr(flag.find('=') + 1);

        int homeCharIndex = relativePath.find('~');
        if (homeCharIndex != relativePath.npos) {
          relativePath.insert(homeCharIndex + 1, getenv("HOME"));
          relativePath.erase(homeCharIndex, 1);
        }

        AdvancedWebserver::DATA_DIR = std::filesystem::absolute(relativePath);
      }
    }
  }

  if (p["uri"].empty() || p["uri"].size() > 1) {
    std::cerr << "Incorrect URI given to the program" << std::endl;
    return 1;
  }

  if (p["type"].empty() || p["type"].size() > 1) {
    std::cerr << "Incorrect configuration type given to the program"
              << std::endl;
    return 1;
  }

  if (p["file"].empty() || p["file"].size() > 1) {
    std::cerr << "Incorrect file/program path" << std::endl;
    return 1;
  }

  if (p["http_file_type"].empty() || p["http_file_type"].size() > 1) {
    std::cerr << "Incorrect file/program type" << std::endl;
    return 1;
  }

  for (const auto &conf : AdvancedWebserver::ConfigurationTypes) {
    if (conf.GetType() == p["type"][0]) {
      validConfigurationType = true;
    }
  }

  if (!validConfigurationType) {
    std::cerr << "Incorrect configuration type given to the program. See "
                 "--help for valid values"
              << std::endl;
    return 1;
  }

  AdvancedWebserver::Configuration c(p["uri"][0]);

  for (const auto &confType : AdvancedWebserver::ConfigurationTypes) {
    if (p["type"][0] == confType.GetType()) {
      c.SetConfigurationType(confType);
    }
  }

  c.SetPath(p["file"][0]);
  c.SetFileType(p["http_file_type"][0]);

  if (c.WriteFile(AdvancedWebserver::DATA_DIR)) {
    std::cout << "Configuration file created successfully for " << p["uri"][0]
              << std::endl;
    return 0;
  } else {
    std::cerr << "Failed to generate configuration file for " << p["uri"][0]
              << std::endl;
    return 1;
  }
}
