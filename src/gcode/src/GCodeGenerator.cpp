#include "owl/gcode/GCodeGenerator.hpp"

#include <iomanip>
#include <sstream>

namespace owl::gcode
{
std::vector<std::string> GCodeGenerator::Build(const Job& job) const
{
    std::vector<std::string> lines;
    lines.emplace_back("G90");
    lines.emplace_back("G21");

    if (job.mode == ProcessMode::Laser)
    {
        lines.emplace_back("M4");
    }

    for (const MotionPoint& point : job.points)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3)
            << "G1 X" << point.x
            << " Y" << point.y
            << " Z" << point.z
            << " F" << std::setprecision(1) << point.feed
            << " S" << point.power;
        lines.emplace_back(oss.str());
    }

    if (job.mode == ProcessMode::Laser)
    {
        lines.emplace_back("M5");
    }
    lines.emplace_back("M2");
    return lines;
}
}
