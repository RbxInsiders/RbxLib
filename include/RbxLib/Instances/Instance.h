#pragma once

#include <Winux/Winux.h>

#include <memory>

namespace RbxLib::Roblox {

class RobloxClient;

class Instance {
public:
    Instance();
    Instance(const Instance& other) = default;
    Instance(Instance&& other) noexcept = default;
    Instance& operator=(const Instance& other) = default;
    Instance& operator=(Instance&& other) noexcept = default;

    [[nodiscard]] bool IsValid() const noexcept;

protected:
    explicit Instance(std::shared_ptr<void> session);

    [[nodiscard]] std::shared_ptr<void> Session() const noexcept;

private:
    std::shared_ptr<void> m_session;
};

}
