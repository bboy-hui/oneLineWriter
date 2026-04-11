#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace owl::gcode
{
bool WriteGCodeFile(const std::filesystem::path& filePath, const std::vector<std::string>& lines);
}
