#pragma once
#include <filesystem>
#include <string>

namespace AdvancedWebserver {
static constexpr const char VERSION[] = "V0.1";

static std::filesystem::path DATA_DIR =
    (std::string)getenv("HOME") + "/.local/share/AdvancedWebserver";

} // namespace AdvancedWebserver
