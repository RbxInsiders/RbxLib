#pragma once

#include <RbxLib/Core/Result.h>

#include <array>
#include <cstdint>
#include <memory>

namespace RbxLib::Roblox {

class BasePart;

class Primitive final {
public:
    Primitive() = default;

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] Core::Result<std::array<float, 3>> Position() const;
    [[nodiscard]] Core::Result<std::array<float, 9>> Rotation() const;
    [[nodiscard]] Core::Result<std::array<float, 3>> Size() const;
    [[nodiscard]] Core::Result<std::array<float, 3>> AssemblyLinearVelocity() const;
    [[nodiscard]] Core::Result<std::array<float, 3>> AssemblyAngularVelocity() const;
    [[nodiscard]] Core::Result<std::uint32_t> Flags() const;
    [[nodiscard]] Core::Result<bool> IsAnchored() const;
    [[nodiscard]] Core::Result<bool> CanCollide() const;
    [[nodiscard]] Core::Result<bool> CanTouch() const;
    [[nodiscard]] Core::Result<bool> CanQuery() const;

private:
    friend class BasePart;

    Primitive(std::shared_ptr<void> session, std::uintptr_t address);

    std::shared_ptr<void> m_session;
    std::uintptr_t m_address = 0;
};

}