#include "owl/core/Application.hpp"

#include <utility>

#include <spdlog/spdlog.h>

namespace owl::core
{
Application::Application(std::string name)
    : name_(std::move(name))
{
}

void Application::Bootstrap()
{
    spdlog::info("Application bootstrapped: {}", name_);
}

const std::string& Application::Name() const noexcept
{
    return name_;
}
}
