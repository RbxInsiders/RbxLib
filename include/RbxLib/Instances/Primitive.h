#pragma once

#include <Winux/Winux.h>

#include <array>
#include <cstdint>
#include <memory>

namespace RbxLib::Roblox {

class BasePart;

class Primitive final {
public:
    Primitive() = default;

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] Winux::Core::Result<std::array<float, 3>> Position() const;
    [[nodiscard]] Winux::Core::Result<std::array<float, 9>> Rotation() const;
    [[nodiscard]] Winux::Core::Result<std::array<float, 3>> Size() const;
    [[nodiscard]] Winux::Core::Result<std::array<float, 3>> AssemblyLinearVelocity() const;
    [[nodiscard]] Winux::Core::Result<std::array<float, 3>> AssemblyAngularVelocity() const;
    [[nodiscard]] Winux::Core::Result<std::uint32_t> Flags() const;
    [[nodiscard]] Winux::Core::Result<bool> IsAnchored() const;
    [[nodiscard]] Winux::Core::Result<bool> CanCollide() const;
    [[nodiscard]] Winux::Core::Result<bool> CanTouch() const;
    [[nodiscard]] Winux::Core::Result<bool> CanQuery() const;

private:
    friend class BasePart;

    Primitive(std::shared_ptr<void> session, std::uintptr_t address);

    std::shared_ptr<void> m_session;
    std::uintptr_t m_address = 0;
};

}