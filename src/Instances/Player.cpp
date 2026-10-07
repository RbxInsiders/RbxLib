#include <RbxLib/Instances/Player.h>

#include <RbxLib/Client/RobloxClient.h>

namespace RbxLib::Roblox {

Player::Player(const Instance& instance)
    : Instance(instance) {}

Winux::Core::Result<Player> Player::FromInstance(const Instance& instance) {
    if (!instance.IsValid()) {
        return Winux::Core::Result<Player>::Failure("Instance is not valid");
    }

    const auto isPlayer = instance.IsA("Player");
    if (!isPlayer.Succeeded()) {
        return Winux::Core::Result<Player>::Failure(isPlayer.Message());
    }
    if (!isPlayer.Value()) {
        return Winux::Core::Result<Player>::Failure("Instance is not a Player");
    }
    return Winux::Core::Result<Player>::Success(Player(instance));
}

Winux::Core::Result<std::uint32_t> Player::UserId() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Winux::Core::Result<std::uint32_t>::Failure("Player session is unavailable");
    }
    return ReadUInt32Field(session->PlayerUserId, "Player.UserId");
}

Winux::Core::Result<std::string> Player::DisplayName() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Winux::Core::Result<std::string>::Failure("Player session is unavailable");
    }
    return ReadStringField(session->PlayerDisplayName, "Player.DisplayName");
}

Winux::Core::Result<std::int32_t> Player::AccountAge() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Winux::Core::Result<std::int32_t>::Failure("Player session is unavailable");
    }
    return ReadInt32Field(session->PlayerAccountAge, "Player.AccountAge");
}

}