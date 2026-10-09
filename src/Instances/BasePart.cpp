#include <RbxLib/Instances/BasePart.h>

#include <RbxLib/Client/RobloxClient.h>

namespace RbxLib::Roblox {

BasePart::BasePart(const Instance& instance)
    : Instance(instance) {}

Winux::Core::Result<BasePart> BasePart::FromInstance(const Instance& instance) {
    if (!instance.IsValid()) {
        return Winux::Core::Result<BasePart>::Failure("Instance is not valid");
    }

    const auto isBasePart = instance.IsA("BasePart");
    if (!isBasePart.Succeeded()) {
        return Winux::Core::Result<BasePart>::Failure(isBasePart.Message());
    }
    if (!isBasePart.Value()) {
        return Winux::Core::Result<BasePart>::Failure("Instance is not a BasePart");
    }
    return Winux::Core::Result<BasePart>::Success(BasePart(instance));
}

Winux::Core::Result<Primitive> BasePart::GetPrimitive() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Winux::Core::Result<Primitive>::Failure("BasePart session is unavailable");
    }

    const auto address = ReadPointerField(session->BasePartPrimitive, "BasePart.Primitive");
    if (!address.Succeeded()) {
        return Winux::Core::Result<Primitive>::Failure(address.Message());
    }
    if (address.Value() == 0) {
        return Winux::Core::Result<Primitive>::Failure("BasePart.Primitive is null");
    }
    return Winux::Core::Result<Primitive>::Success(Primitive(Session(), address.Value()));
}

}