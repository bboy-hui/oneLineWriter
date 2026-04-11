#include "DesktopWorkbench.hpp"

#include <algorithm>
#include <string_view>
#include <utility>

#include <imgui.h>

#include "owl/gcode/GCodeGenerator.hpp"
#include "owl/gcode/GCodeIo.hpp"
#include "owl/gcode/TextStrokePlanner.hpp"

namespace owl::desktop
{
DesktopWorkbench::DesktopWorkbench()
{
    const std::string_view defaultText = "HELLO GRBL";
    std::copy(defaultText.begin(), defaultText.end(), textBuffer_.begin());

    const std::string_view defaultPath = "output/demo.gcode";
    std::copy(defaultPath.begin(), defaultPath.end(), outputPathBuffer_.begin());
}

bool DesktopWorkbench::BuildAndExport()
{
    owl::gcode::TextStrokePlanner planner;
    owl::gcode::TextPlanOptions options;
    options.startX = 0.0f;
    options.startY = 20.0f;

    lastJob_ = planner.BuildPenJob(textBuffer_.data(), options);
    owl::gcode::GCodeGenerator generator;
    lastGCode_ = generator.Build(lastJob_);

    outputPath_ = outputPathBuffer_.data();
    const bool written = owl::gcode::WriteGCodeFile(outputPath_, lastGCode_);
    status_ = written ? "导出成功" : "导出失败";
    return written;
}

void DesktopWorkbench::DrawPanels()
{
    ImGui::Begin("OneLineWriter");
    ImGui::InputText("Text", textBuffer_.data(), textBuffer_.size());
    ImGui::InputText("GCode Path", outputPathBuffer_.data(), outputPathBuffer_.size());
    if (ImGui::Button("Build And Export"))
    {
        BuildAndExport();
    }
    ImGui::TextUnformatted(status_.c_str());
    ImGui::Separator();
    ImGui::Text("Preview Lines: %d", static_cast<int>(lastGCode_.size()));
    ImGui::End();
}

const std::vector<std::string>& DesktopWorkbench::LastGCode() const noexcept
{
    return lastGCode_;
}

std::vector<owl::render::Polyline3D> DesktopWorkbench::BuildPreviewPolylines() const
{
    constexpr float upThreshold = 0.001f;
    constexpr std::uint32_t processColor = 0x44CCFFFFu;
    constexpr std::uint32_t travelColor = 0x88999999u;

    std::vector<owl::render::Polyline3D> result;
    owl::render::Polyline3D processLine;
    processLine.rgba = processColor;
    processLine.layer = owl::render::Polyline3D::Layer::Process;
    owl::render::Polyline3D travelLine;
    travelLine.rgba = travelColor;
    travelLine.layer = owl::render::Polyline3D::Layer::Travel;

    for (const auto& point : lastJob_.points)
    {
        const bool isTravelPoint = point.z > upThreshold;
        if (isTravelPoint)
        {
            if (!processLine.points.empty())
            {
                result.emplace_back(std::move(processLine));
                processLine = owl::render::Polyline3D {};
                processLine.rgba = processColor;
                processLine.layer = owl::render::Polyline3D::Layer::Process;
            }
            travelLine.points.push_back({point.x, point.y, point.z});
        }
        else
        {
            if (!travelLine.points.empty())
            {
                result.emplace_back(std::move(travelLine));
                travelLine = owl::render::Polyline3D {};
                travelLine.rgba = travelColor;
                travelLine.layer = owl::render::Polyline3D::Layer::Travel;
            }
            processLine.points.push_back({point.x, point.y, point.z});
        }
    }

    if (!processLine.points.empty())
    {
        result.emplace_back(std::move(processLine));
    }
    if (!travelLine.points.empty())
    {
        result.emplace_back(std::move(travelLine));
    }

    return result;
}

const std::string& DesktopWorkbench::OutputPath() const noexcept
{
    return outputPath_;
}
}
