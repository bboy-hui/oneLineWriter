#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace owl::render
{
enum class BackendType
{
    Vulkan,
    OpenGL
};

struct Vec3
{
    float x {};
    float y {};
    float z {};
};

struct CameraState
{
    Vec3 eye {};
    Vec3 target {};
    float fovYDeg {45.0f};
};

struct Polyline3D
{
    enum class Layer
    {
        Process,
        Travel
    };

    std::vector<Vec3> points;
    std::uint32_t rgba {0xFFFFFFFFu};
    Layer layer {Layer::Process};
};

struct Aabb3
{
    Vec3 min {};
    Vec3 max {};
    bool valid {false};
};

struct RenderFrameStats
{
    std::size_t polylineCount {0};
    std::size_t processPolylineCount {0};
    std::size_t travelPolylineCount {0};
    std::size_t vertexCount {0};
    Aabb3 bounds {};
};
}
