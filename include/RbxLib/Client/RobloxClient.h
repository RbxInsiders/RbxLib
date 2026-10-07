#pragma once

#include <RbxLib/Instances/DataModel.h>
#include <RbxLib/Offsets/OffsetProvider.h>

#include <Winux/Winux.h>

#include <cstdint>
#include <filesystem>
#include <memory>

namespace RbxLib::Roblox {

class RobloxClient final {
public:
    RobloxClient(
        std::uint32_t processId,
        Winux::Process& process,
        Winux::Memory& memory,
        const Offsets::OffsetProvider& offsets);

    RobloxClient(const RobloxClient&) = delete;
    RobloxClient(RobloxClient&&) noexcept = delete;
    RobloxClient& operator=(const RobloxClient&) = delete;
    RobloxClient& operator=(RobloxClient&&) noexcept = delete;

    [[nodiscard]] Winux::Core::Result<DataModel> GetDataModel() const;
    [[nodiscard]] std::uint32_t ProcessId() const noexcept;

public:
    struct Session {
        std::uint32_t ProcessId = 0;
        Winux::Process* Process = nullptr;
        Winux::Memory* Memory = nullptr;
        const Offsets::OffsetProvider* Offsets = nullptr;
        std::uintptr_t DataModelAddress = 0;
        std::uintptr_t FakeDataModelPointer = 0;
        std::uintptr_t FakeDataModelRealDataModel = 0;
        std::uintptr_t DataModelPlaceId = 0;
        std::uintptr_t DataModelGameId = 0;
        std::uintptr_t DataModelCreatorId = 0;
        std::uintptr_t DataModelGameLoaded = 0;
        std::uintptr_t InstanceNameContainer = 0;
        std::uintptr_t InstanceName = 0;
        std::uintptr_t InstanceClassDescriptor = 0;
        std::uintptr_t InstanceParent = 0;
        std::uintptr_t InstanceChildrenStart = 0;
        std::uintptr_t InstanceChildrenEnd = 0;
        std::uintptr_t InstanceClassName = 0;
        std::uintptr_t InstanceClassBase = 0;
        std::uintptr_t PlayerUserId = 0;
        std::uintptr_t PlayerDisplayName = 0;
        std::uintptr_t PlayerAccountAge = 0;
    };

private:
    std::shared_ptr<Session> m_session;
};

}
