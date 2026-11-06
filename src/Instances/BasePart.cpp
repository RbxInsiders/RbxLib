#include <RbxLib/Instances/BasePart.h>

#include <RbxLib/Client/RobloxClient.h>

namespace RbxLib::Roblox {

BasePart::BasePart(const Instance& instance)
    : Instance(instance) {}

Core::Result<BasePart> BasePart::FromInstance(const Instance& instance) {
    if (!instance.IsValid()) {
        return Core::Result<BasePart>::Failure("Instance is not valid");
    }

    const auto isBasePart = instance.IsA("BasePart");
    if (!isBasePart.Succeeded()) {
        return Core::Result<BasePart>::Failure(isBasePart.Message());
    }
    if (!isBasePart.Value()) {
        return Core::Result<BasePart>::Failure("Instance is not a BasePart");
    }
    return Core::Result<BasePart>::Success(BasePart(instance));
}

Core::Result<Primitive> BasePart::GetPrimitive() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(Session());
    if (!session) {
        return Core::Result<Primitive>::Failure("BasePart session is unavailable");
    }

    const auto address = ReadPointerField(session->BasePartPrimitive, "BasePart.Primitive");
    if (!address.Succeeded()) {
        return Core::Result<Primitive>::Failure(address.Message());
    }
    if (address.Value() == 0) {
        return Core::Result<Primitive>::Failure("BasePart.Primitive is null");
    }
    return Core::Result<Primitive>::Success(Primitive(Session(), address.Value()));
}

}