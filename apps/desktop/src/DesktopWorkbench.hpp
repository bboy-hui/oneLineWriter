#pragma once

#include <array>
#include <string>
#include <vector>

#include "owl/gcode/GCodeGenerator.hpp"
#include "owl/render/RenderTypes.hpp"

namespace owl::desktop
{
class DesktopWorkbench
{
public:
    DesktopWorkbench();
    bool BuildAndExport();
    void DrawPanels();
    [[nodiscard]] const std::vector<std::string>& LastGCode() const noexcept;
    [[nodiscard]] std::vector<owl::render::Polyline3D> BuildPreviewPolylines() const;
    [[nodiscard]] const std::string& OutputPath() const noexcept;

private:
    std::array<char, 128> textBuffer_ {};
    std::array<char, 260> outputPathBuffer_ {};
    owl::gcode::Job lastJob_ {};
    std::vector<std::string> lastGCode_;
    std::string outputPath_;
    std::string status_;
};
}
