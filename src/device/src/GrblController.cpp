#include "owl/device/GrblController.hpp"

#include <algorithm>
#include <utility>

namespace owl::device
{
bool GrblController::Connect(const std::string&, int)
{
    connected_ = true;
    state_ = ControllerState::Idle;
    return connected_;
}

void GrblController::Disconnect()
{
    connected_ = false;
    state_ = ControllerState::Disconnected;
    pendingLines_.clear();
}

bool GrblController::IsConnected() const noexcept
{
    return connected_;
}

ControllerState GrblController::State() const noexcept
{
    return state_;
}

void GrblController::QueueGCode(std::vector<std::string> lines)
{
    for (std::string& line : lines)
    {
        pendingLines_.emplace_back(std::move(line));
    }
}

std::vector<std::string> GrblController::FlushPending(std::size_t maxLines)
{
    const std::size_t count = std::min(maxLines, pendingLines_.size());
    std::vector<std::string> chunk;
    chunk.reserve(count);
    for (std::size_t index = 0; index < count; ++index)
    {
        chunk.emplace_back(std::move(pendingLines_.front()));
        pendingLines_.pop_front();
    }
    if (connected_)
    {
        state_ = pendingLines_.empty() ? ControllerState::Idle : ControllerState::Run;
    }
    return chunk;
}
}
