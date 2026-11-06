#pragma once

#include <RbxLib/Core/Result.h>
#include <RbxLib/Instances/DataModel.h>
#include <RbxLib/Offsets/OffsetProvider.h>
#include <RbxLib/Rendering/VisualEngine.h>

#include <cstdint>
#include <memory>

namespace RbxLib::Roblox {

class RobloxClient final {
public:
    RobloxClient(std::uint32_t processId, const Offsets::OffsetProvider& offsets);

    RobloxClient(const RobloxClient&) = delete;
    RobloxClient(RobloxClient&&) noexcept = delete;
    RobloxClient& operator=(const RobloxClient&) = delete;
    RobloxClient& operator=(RobloxClient&&) noexcept = delete;

    [[nodiscard]] Core::Result<DataModel> GetDataModel() const;
    [[nodiscard]] Core::Result<Rendering::VisualEngine> GetVisualEngine() const;
    [[nodiscard]] std::uint32_t ProcessId() const noexcept;

public:
    struct Session {
        std::uint32_t ProcessId = 0;
        const Offsets::OffsetProvider* Offsets = nullptr;
        std::uintptr_t DataModelAddress = 0;
        std::uintptr_t FakeDataModelPointer = 0;
        std::uintptr_t FakeDataModelRealDataModel = 0;
        std::uintptr_t DataModelPlaceId = 0;
        std::uintptr_t DataModelGameId = 0;
        std::uintptr_t DataModelCreatorId = 0;
        std::uintptr_t DataModelGameLoaded = 0;
        std::uintptr_t VisualEnginePointer = 0;
        std::uintptr_t VisualEngineDimensions = 0;
        std::uintptr_t VisualEngineViewMatrix = 0;
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
        std::uintptr_t PlayerModelInstance = 0;
        std::uintptr_t BasePartPrimitive = 0;
        std::uintptr_t PrimitiveAssemblyAngularVelocity = 0;
        std::uintptr_t PrimitiveAssemblyLinearVelocity = 0;
        std::uintptr_t PrimitiveFlags = 0;
        std::uintptr_t PrimitivePosition = 0;
        std::uintptr_t PrimitiveRotation = 0;
        std::uintptr_t PrimitiveSize = 0;
    };

private:
    std::shared_ptr<Session> m_session;
};

}
