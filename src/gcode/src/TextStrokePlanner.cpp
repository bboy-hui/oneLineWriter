#include "owl/gcode/TextStrokePlanner.hpp"

#include <cctype>

namespace owl::gcode
{
namespace
{
void AppendRectGlyph(Job& job, float left, float baselineY, const TextPlanOptions& options)
{
    const float right = left + options.charWidth;
    const float top = baselineY;
    const float bottom = baselineY - options.charHeight;

    job.points.push_back({left, top, options.penUpZ, options.feed, 0.0f});
    job.points.push_back({left, top, options.penDownZ, options.feed, 0.0f});
    job.points.push_back({right, top, options.penDownZ, options.feed, 0.0f});
    job.points.push_back({right, bottom, options.penDownZ, options.feed, 0.0f});
    job.points.push_back({left, bottom, options.penDownZ, options.feed, 0.0f});
    job.points.push_back({left, top, options.penDownZ, options.feed, 0.0f});
    job.points.push_back({left, top, options.penUpZ, options.feed, 0.0f});
}
}

Job TextStrokePlanner::BuildPenJob(const std::string& text, const TextPlanOptions& options) const
{
    Job job;
    job.mode = ProcessMode::Pen;

    float cursorX = options.startX;
    const float baselineY = options.startY;

    for (const char raw : text)
    {
        if (raw == ' ')
        {
            cursorX += options.charWidth + options.spacing;
            continue;
        }

        const unsigned char ch = static_cast<unsigned char>(raw);
        if (!std::isprint(ch))
        {
            continue;
        }

        AppendRectGlyph(job, cursorX, baselineY, options);
        cursorX += options.charWidth + options.spacing;
    }

    return job;
}
}
