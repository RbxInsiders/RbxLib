#pragma once

#include <RbxLib/Core/Result.h>
#include <RbxLib/Instances/Instance.h>

#include <cstdint>
#include <string>

namespace RbxLib::Roblox {

class Player final : public Instance {
public:
    Player() = default;
    Player(const Player& other) = default;
    Player(Player&& other) noexcept = default;
    Player& operator=(const Player& other) = default;
    Player& operator=(Player&& other) noexcept = default;

    [[nodiscard]] static Core::Result<Player> FromInstance(
        const Instance& instance);

    [[nodiscard]] Core::Result<std::uint32_t> UserId() const;
    [[nodiscard]] Core::Result<std::string> DisplayName() const;
    [[nodiscard]] Core::Result<std::int32_t> AccountAge() const;
    [[nodiscard]] Core::Result<Instance> Character() const;

private:
    explicit Player(const Instance& instance);
};

}