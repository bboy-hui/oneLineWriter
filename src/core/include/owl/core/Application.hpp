#pragma once

#include <string>

namespace owl::core
{
class Application
{
public:
    explicit Application(std::string name);
    void Bootstrap();
    [[nodiscard]] const std::string& Name() const noexcept;

private:
    std::string name_;
};
}
