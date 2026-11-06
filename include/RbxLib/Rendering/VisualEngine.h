#pragma once

#include <RbxLib/Rendering/RenderView.h>

#include <cstdint>
#include <memory>

namespace RbxLib::Roblox {
class RobloxClient;
}

namespace RbxLib::Rendering {

class VisualEngine final {
public:
    VisualEngine() = default;
    VisualEngine(const VisualEngine& other) = default;
    VisualEngine(VisualEngine&& other) noexcept = default;
    VisualEngine& operator=(const VisualEngine& other) = default;
    VisualEngine& operator=(VisualEngine&& other) noexcept = default;

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] Core::Result<RenderView> Snapshot() const;

private:
    friend class Roblox::RobloxClient;

    VisualEngine(std::shared_ptr<void> session, std::uintptr_t address);

    std::shared_ptr<void> m_session;
    std::uintptr_t m_address = 0;
};

}