#pragma once

#include <deque>
#include <string>
#include <vector>

namespace owl::device
{
enum class ControllerState
{
    Disconnected,
    Idle,
    Run,
    Hold,
    Alarm
};

class GrblController
{
public:
    bool Connect(const std::string& portName, int baudRate);
    void Disconnect();
    [[nodiscard]] bool IsConnected() const noexcept;
    [[nodiscard]] ControllerState State() const noexcept;
    void QueueGCode(std::vector<std::string> lines);
    std::vector<std::string> FlushPending(std::size_t maxLines);

private:
    bool connected_ {false};
    ControllerState state_ {ControllerState::Disconnected};
    std::deque<std::string> pendingLines_;
};
}
