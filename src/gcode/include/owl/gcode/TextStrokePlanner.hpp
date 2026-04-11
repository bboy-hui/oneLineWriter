#pragma once

#include <string>

#include "owl/gcode/GCodeGenerator.hpp"

namespace owl::gcode
{
struct TextPlanOptions
{
    float startX {0.0f};
    float startY {0.0f};
    float charWidth {6.0f};
    float charHeight {8.0f};
    float spacing {2.0f};
    float penUpZ {5.0f};
    float penDownZ {0.0f};
    float feed {1200.0f};
};

class TextStrokePlanner
{
public:
    Job BuildPenJob(const std::string& text, const TextPlanOptions& options) const;
};
}
