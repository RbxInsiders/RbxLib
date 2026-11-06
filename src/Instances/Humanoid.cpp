#include <RbxLib/Instances/Humanoid.h>

#include <RbxLib/Client/RobloxClient.h>

namespace RbxLib::Roblox {

Humanoid::Humanoid(const Instance& instance)
    : Instance(instance) {}

Core::Result<Humanoid> Humanoid::FromInstance(const Instance& instance) {
    if (!instance.IsValid()) {
        return Core::Result<Humanoid>::Failure("Instance is not valid");
    }

    const auto isHumanoid = instance.IsA("Humanoid");
    if (!isHumanoid.Succeeded()) {
        return Core::Result<Humanoid>::Failure(isHumanoid.Message());
    }
    if (!isHumanoid.Value()) {
        return Core::Result<Humanoid>::Failure("Instance is not a Humanoid");
    }
    return Core::Result<Humanoid>::Success(Humanoid(instance));
}

Core::Result<float> Humanoid::Health() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Core::Result<float>::Failure("Humanoid session is unavailable");
    }
    return ReadFloatField(session->HumanoidHealth, "Humanoid.Health");
}

Core::Result<float> Humanoid::MaxHealth() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Core::Result<float>::Failure("Humanoid session is unavailable");
    }
    return ReadFloatField(session->HumanoidMaxHealth, "Humanoid.MaxHealth");
}

}