#include <RbxLib/Instances/Player.h>

#include <RbxLib/Client/RobloxClient.h>

namespace RbxLib::Roblox {

Player::Player(const Instance& instance)
    : Instance(instance) {}

Core::Result<Player> Player::FromInstance(const Instance& instance) {
    if (!instance.IsValid()) {
        return Core::Result<Player>::Failure("Instance is not valid");
    }

    const auto isPlayer = instance.IsA("Player");
    if (!isPlayer.Succeeded()) {
        return Core::Result<Player>::Failure(isPlayer.Message());
    }
    if (!isPlayer.Value()) {
        return Core::Result<Player>::Failure("Instance is not a Player");
    }
    return Core::Result<Player>::Success(Player(instance));
}

Core::Result<std::uint32_t> Player::UserId() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Core::Result<std::uint32_t>::Failure("Player session is unavailable");
    }
    return ReadUInt32Field(session->PlayerUserId, "Player.UserId");
}

Core::Result<std::string> Player::DisplayName() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Core::Result<std::string>::Failure("Player session is unavailable");
    }
    return ReadStringField(session->PlayerDisplayName, "Player.DisplayName");
}

Core::Result<std::int32_t> Player::AccountAge() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Core::Result<std::int32_t>::Failure("Player session is unavailable");
    }
    return ReadInt32Field(session->PlayerAccountAge, "Player.AccountAge");
}

Core::Result<Instance> Player::Character() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Core::Result<Instance>::Failure("Player session is unavailable");
    }

    const auto address = ReadPointerField(
        session->PlayerModelInstance, "Player.ModelInstance");
    if (!address.Succeeded()) {
        return Core::Result<Instance>::Failure(address.Message());
    }
    if (address.Value() == 0) {
        return Core::Result<Instance>::Success(Instance{});
    }
    return Core::Result<Instance>::Success(AtAddress(address.Value()));
}

}