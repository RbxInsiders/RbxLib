#include <RbxLib/Instances/DataModel.h>

#include <RbxLib/Client/RobloxClient.h>

#include <Winux/Winux.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

namespace RbxLib::Roblox {
namespace {

Winux::Core::Result<std::uint64_t> ReadField(
    const std::shared_ptr<void>& session,
    const std::uintptr_t offset,
    const std::string& fieldName) {
    const auto clientSession = std::static_pointer_cast<RobloxClient::Session>(session);
    if (!clientSession || !clientSession->Process || !clientSession->Memory) {
        return Winux::Core::Result<std::uint64_t>::Failure(
            "DataModel session is no longer available");
    }

    std::array<std::byte, sizeof(std::uint64_t)> buffer{};
    const auto read = clientSession->Memory->ReadMemory(
        clientSession->ProcessId,
        clientSession->DataModelAddress + offset,
        buffer);
    if (!read.Succeeded()) {
        return Winux::Core::Result<std::uint64_t>::Failure(
            "Unable to read DataModel." + fieldName + ": " + read.Message());
    }

    std::uint64_t result = 0;
    std::memcpy(&result, buffer.data(), sizeof(result));
    return Winux::Core::Result<std::uint64_t>::Success(result);
}

}

DataModel::DataModel() = default;

DataModel::DataModel(std::shared_ptr<void> session, const std::uintptr_t address)
    : Instance(std::move(session), address) {}

Winux::Core::Result<std::uint64_t> DataModel::PlaceId() const {
    return ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelPlaceId,
        "PlaceId");
}

Winux::Core::Result<std::uint64_t> DataModel::GameId() const {
    return ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelGameId,
        "GameId");
}

Winux::Core::Result<std::uint64_t> DataModel::CreatorId() const {
    return ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelCreatorId,
        "CreatorId");
}

Winux::Core::Result<bool> DataModel::GameLoaded() const {
    const auto loaded = ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelGameLoaded,
        "GameLoaded");
    if (!loaded.Succeeded()) {
        return Winux::Core::Result<bool>::Failure(loaded.Message());
    }
    return Winux::Core::Result<bool>::Success(loaded.Value() == 31);
}

}

