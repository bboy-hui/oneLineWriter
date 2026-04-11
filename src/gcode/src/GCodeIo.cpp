#include "owl/gcode/GCodeIo.hpp"

#include <fstream>
#include <system_error>

namespace owl::gcode
{
bool WriteGCodeFile(const std::filesystem::path& filePath, const std::vector<std::string>& lines)
{
    const auto parent = filePath.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
    }

    std::ofstream output(filePath, std::ios::out | std::ios::trunc);
    if (!output.is_open())
    {
        return false;
    }

    for (const std::string& line : lines)
    {
        output << line << '\n';
    }

    return output.good();
}
}
