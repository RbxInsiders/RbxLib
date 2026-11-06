#pragma once

#include <RbxLib/Core/Result.h>
#include <RbxLib/Instances/Instance.h>

namespace RbxLib::Roblox {

class Humanoid final : public Instance {
public:
    Humanoid() = default;
    Humanoid(const Humanoid& other) = default;
    Humanoid(Humanoid&& other) noexcept = default;
    Humanoid& operator=(const Humanoid& other) = default;
    Humanoid& operator=(Humanoid&& other) noexcept = default;

    [[nodiscard]] static Core::Result<Humanoid> FromInstance(
        const Instance& instance);

    [[nodiscard]] Core::Result<float> Health() const;
    [[nodiscard]] Core::Result<float> MaxHealth() const;

private:
    explicit Humanoid(const Instance& instance);
};

}