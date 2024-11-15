#include "Core/Core.h"
#include "GArgs/GArgs.hpp"
#include "Server/HandleConnection.h"
#include "Server/ServerApp.h"
#include <filesystem>
#include <iostream>
#include <string>

static constexpr const char ADDRESS[] = "0.0.0.0";
static constexpr const int PORT = 8081;

int main(int argc, char *argv[]) {
  std::string address = ADDRESS;
  int port = PORT;

  GArgs::Parser p("AdvancedWebserver", "V1.0");
  p.AddStructure(
      "[flags:help=Any flags to pass the "
      "application,argument_filter=-,value_amount=0;cert:help=The SSL "
      "certificate to use;key:help=The SSL key for the certificate]");

  p.AddKey(GArgs::Key("flags", "--help | -h", "Display this message"));
  p.AddKey(GArgs::Key("flags", "--address=0.0.0.0",
                      "Sets the address to listen on"));
  p.AddKey(GArgs::Key("flags", "--port=8081", "Sets the port to listen on"));
  p.AddKey(GArgs::Key("flags",
                      "--set-data-dir=$HOME/.local/share/AdvancedWebserver/",
                      "Set the directory where the Webserver data is stored"));

  p.AddKey(GArgs::Key("cert", "* | any path",
                      "Set the SSL Certificate path for the server"));
  p.AddKey(
      GArgs::Key("key", "* | any path", "Set the SSL Key path for the server"));

  p.ParseArgs(argc, argv);

  if (p.Contains("flags", "--help") || p.Contains("flags", "-h")) {
    p.DisplayHelp();
    return 0;
  }

  for (const auto &flag : p["flags"]) {
    if (flag.find("--address=") != flag.npos) {
      address = flag.substr(flag.find('=') + 1);
    }

    if (flag.find("--port=") != flag.npos) {
      port = std::stoi(flag.substr(flag.find('=') + 1));
    }

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

  if (p["cert"].size() < 1 || p["cert"].size() > 1) {
    std::cerr << "SSL Certificate not provided. See --help." << std::endl;
    return 1;
  }

  if (p["key"].size() < 1 || p["key"].size() > 1) {
    std::cerr << "SSL Key not provided. See --help." << std::endl;
    return 1;
  }

  if (!std::filesystem::exists(std::filesystem::absolute(p["cert"][0])) ||
      !std::filesystem::is_regular_file(
          std::filesystem::absolute(p["cert"][0]))) {
    std::cerr << "SSL Certificate not found. Please check the provided path."
              << std::endl;
    return 1;
  }

  if (!std::filesystem::exists(std::filesystem::absolute(p["key"][0])) ||
      !std::filesystem::is_regular_file(
          std::filesystem::absolute(p["key"][0]))) {
    std::cerr << "SSL Key not found. Please check the provided path."
              << std::endl;
    return 1;
  }

  AdvancedWebserver::ServerApp server(
      address, port, AdvancedWebserver::DATA_DIR, p["cert"][0], p["key"][0]);
  std::cout << "Starting server on: " << address << ':' << port << " using "
            << AdvancedWebserver::DATA_DIR << " as data directory" << std::endl;
  server.Run(AdvancedWebserver::HandleConnection);
  return 0;
}
