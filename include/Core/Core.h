#pragma once
#include <filesystem>
#include <string>

namespace AdvancedWebserver {
static std::filesystem::path DATA_DIR =
    (std::string)getenv("HOME") + "/.local/share/AdvancedWebserver";

}
