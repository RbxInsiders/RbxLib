#include <RbxLib/Instances/DataModel.h>

#include <RbxLib/Client/RobloxClient.h>
#include <RbxLib/Core/Memory.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

namespace RbxLib::Roblox {
namespace {

Core::Result<std::uint64_t> ReadField(
    const std::shared_ptr<void>& session,
    const std::uintptr_t offset,
    const std::string& fieldName) {
    const auto clientSession = std::static_pointer_cast<RobloxClient::Session>(session);
    if (!clientSession) {
        return Core::Result<std::uint64_t>::Failure(
            "DataModel session is no longer available");
    }

    std::array<std::byte, sizeof(std::uint64_t)> buffer{};
    const auto read = Core::Memory::Read(
        clientSession->ProcessId, clientSession->DataModelAddress + offset, buffer);
    if (!read.Succeeded()) {
        return Core::Result<std::uint64_t>::Failure(
            "Unable to read DataModel." + fieldName + ": " + read.Message());
    }

    std::uint64_t result = 0;
    std::memcpy(&result, buffer.data(), sizeof(result));
    return Core::Result<std::uint64_t>::Success(result);
}

}

DataModel::DataModel() = default;

DataModel::DataModel(std::shared_ptr<void> session, const std::uintptr_t address)
    : Instance(std::move(session), address) {}

Core::Result<std::uint64_t> DataModel::PlaceId() const {
    return ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelPlaceId,
        "PlaceId");
}

Core::Result<std::uint64_t> DataModel::GameId() const {
    return ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelGameId,
        "GameId");
}

Core::Result<std::uint64_t> DataModel::CreatorId() const {
    return ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelCreatorId,
        "CreatorId");
}

Core::Result<bool> DataModel::GameLoaded() const {
    const auto loaded = ReadField(
        Session(),
        static_cast<RobloxClient::Session*>(Session().get())->DataModelGameLoaded,
        "GameLoaded");
    if (!loaded.Succeeded()) {
        return Core::Result<bool>::Failure(loaded.Message());
    }
    return Core::Result<bool>::Success(loaded.Value() == 31);
}

}

