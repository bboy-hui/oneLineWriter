#pragma once

#include <span>

#include "owl/render/RenderTypes.hpp"

namespace owl::render
{
inline void ExpandBounds(Aabb3& bounds, const Vec3& point)
{
    if (!bounds.valid)
    {
        bounds.min = point;
        bounds.max = point;
        bounds.valid = true;
        return;
    }

    bounds.min.x = point.x < bounds.min.x ? point.x : bounds.min.x;
    bounds.min.y = point.y < bounds.min.y ? point.y : bounds.min.y;
    bounds.min.z = point.z < bounds.min.z ? point.z : bounds.min.z;
    bounds.max.x = point.x > bounds.max.x ? point.x : bounds.max.x;
    bounds.max.y = point.y > bounds.max.y ? point.y : bounds.max.y;
    bounds.max.z = point.z > bounds.max.z ? point.z : bounds.max.z;
}

inline RenderFrameStats ComputeFrameStats(std::span<const Polyline3D> polylines)
{
    RenderFrameStats stats;
    stats.polylineCount = polylines.size();
    for (const auto& polyline : polylines)
    {
        if (polyline.layer == Polyline3D::Layer::Process)
        {
            ++stats.processPolylineCount;
        }
        else
        {
            ++stats.travelPolylineCount;
        }
        stats.vertexCount += polyline.points.size();
        for (const auto& point : polyline.points)
        {
            ExpandBounds(stats.bounds, point);
        }
    }
    return stats;
}
}
