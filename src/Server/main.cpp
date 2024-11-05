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
  p.AddStructure("[flags:help=Any flags to pass the "
                 "application,argument_filter=-,value_amount=0]");

  p.AddKey(GArgs::Key("flags", "--help | -h", "Display this message"));
  p.AddKey(GArgs::Key("flags", "--address=0.0.0.0",
                      "Sets the address to listen on"));
  p.AddKey(GArgs::Key("flags", "--port=8081", "Sets the port to listen on"));
  p.AddKey(GArgs::Key("flags",
                      "--set-data-dir=$HOME/.local/share/AdvancedWebserver/",
                      "Set the directory where the Webserver data is stored"));

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

  AdvancedWebserver::ServerApp server(address, port,
                                      AdvancedWebserver::DATA_DIR);
  std::cout << "Starting server on: " << address << ':' << port << " using "
            << AdvancedWebserver::DATA_DIR << " as data directory" << std::endl;
  server.Run(AdvancedWebserver::HandleConnection);
  return 0;
}
