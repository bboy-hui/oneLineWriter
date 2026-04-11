#pragma once

#include <string>
#include <vector>

namespace owl::gcode
{
enum class ProcessMode
{
    Pen,
    Laser
};

struct MotionPoint
{
    float x {};
    float y {};
    float z {};
    float feed {};
    float power {};
};

struct Job
{
    ProcessMode mode {ProcessMode::Pen};
    std::vector<MotionPoint> points;
};

class GCodeGenerator
{
public:
    std::vector<std::string> Build(const Job& job) const;
};
}
