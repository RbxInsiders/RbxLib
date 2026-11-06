#include <RbxLib/Client/RobloxClient.h>
#include <RbxLib/Core/Memory.h>
#include <RbxLib/Core/Process.h>

#include <RbxLib/Offsets/OffsetId.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <utility>

namespace RbxLib::Roblox {
namespace {

std::string VersionFromExecutablePath(const std::filesystem::path& executablePath) {
    const auto parent = executablePath.parent_path();
    if (parent.empty()) {
        return {};
    }
    return parent.filename().string();
}

Core::Result<std::uintptr_t> ReadQword(
    std::uint32_t processId,
    const std::uintptr_t address) {
    std::array<std::byte, sizeof(std::uint64_t)> buffer{};
    const auto read = Core::Memory::Read(processId, address, buffer);
    if (!read.Succeeded() || read.Value() != buffer.size()) {
        return Core::Result<std::uintptr_t>::Failure(
            "Unable to read remote address 0x" + std::to_string(address)
            + (read.Succeeded() ? ": short memory read" : ": " + read.Message()));
    }

    std::uintptr_t value = 0;
    std::memcpy(&value, buffer.data(), sizeof(value));
    return Core::Result<std::uintptr_t>::Success(value);
}

Core::Result<std::uintptr_t> ResolveDataModelAddress(
    const std::uint32_t processId,
    const Offsets::OffsetProvider& offsets,
    const std::filesystem::path& executablePath) {
    const auto snapshot = offsets.CurrentVersion();
    if (snapshot.empty()) {
        return Core::Result<std::uintptr_t>::Failure(
            "OffsetProvider snapshot is not ready");
    }

    const auto version = VersionFromExecutablePath(executablePath);
    if (version != snapshot) {
        return Core::Result<std::uintptr_t>::Failure(
            "Executable version \"" + version + "\" does not match offset version \"" + snapshot + "\"");
    }

    const auto fakePointer = offsets.TryGet(Offsets::OffsetId::FakeDataModelPointer);
    const auto fakeRealDataModel = offsets.TryGet(
        Offsets::OffsetId::FakeDataModelRealDataModel);
    if (!fakePointer.has_value() || !fakeRealDataModel.has_value()) {
        return Core::Result<std::uintptr_t>::Failure(
            "OffsetProvider is missing FakeDataModel offsets");
    }

    const auto moduleBase = Core::Process::FindModule(
        executablePath.filename(), processId);
    if (moduleBase.Failed() || moduleBase.Value() == 0) {
        return Core::Result<std::uintptr_t>::Failure(
            moduleBase.Failed()
                ? moduleBase.Message()
                : "Unable to resolve the Roblox module base");
    }

    const auto fakeAddress = ReadQword(
        processId,
        moduleBase.Value() + *fakePointer);
    if (!fakeAddress.Succeeded() || fakeAddress.Value() == 0) {
        return Core::Result<std::uintptr_t>::Failure(
            "Unable to resolve FakeDataModel");
    }

    const auto realAddress = ReadQword(
        processId,
        fakeAddress.Value() + *fakeRealDataModel);
    if (!realAddress.Succeeded() || realAddress.Value() == 0) {
        return Core::Result<std::uintptr_t>::Failure(
            "Unable to resolve the real DataModel");
    }

    return realAddress;
}

Core::Result<std::uintptr_t> ResolveVisualEngineAddress(
    const std::uint32_t processId,
    const Offsets::OffsetProvider& offsets,
    const std::filesystem::path& executablePath) {
    const auto snapshot = offsets.CurrentVersion();
    if (snapshot.empty()) {
        return Core::Result<std::uintptr_t>::Failure(
            "OffsetProvider snapshot is not ready");
    }

    const auto version = VersionFromExecutablePath(executablePath);
    if (version != snapshot) {
        return Core::Result<std::uintptr_t>::Failure(
            "Executable version \"" + version + "\" does not match offset version \"" + snapshot + "\"");
    }

    const auto pointerOffset = offsets.TryGet(Offsets::OffsetId::VisualEnginePointer);
    const auto dimensionsOffset = offsets.TryGet(Offsets::OffsetId::VisualEngineDimensions);
    const auto matrixOffset = offsets.TryGet(Offsets::OffsetId::VisualEngineViewMatrix);
    if (!pointerOffset.has_value() || !dimensionsOffset.has_value() || !matrixOffset.has_value()) {
        return Core::Result<std::uintptr_t>::Failure(
            "OffsetProvider is missing VisualEngine offsets");
    }

    const auto moduleBase = Core::Process::FindModule(
        executablePath.filename(), processId);
    if (moduleBase.Failed() || moduleBase.Value() == 0) {
        return Core::Result<std::uintptr_t>::Failure(
            moduleBase.Failed()
                ? moduleBase.Message()
                : "Unable to resolve the Roblox module base");
    }
    if (moduleBase.Value() > (std::numeric_limits<std::uintptr_t>::max)() - *pointerOffset) {
        return Core::Result<std::uintptr_t>::Failure(
            "Address overflow while resolving VisualEngine");
    }

    const auto visualEngine = ReadQword(
        processId,
        moduleBase.Value() + *pointerOffset);
    if (!visualEngine.Succeeded() || visualEngine.Value() == 0) {
        return Core::Result<std::uintptr_t>::Failure(
            "Unable to resolve VisualEngine");
    }

    return visualEngine;
}
}

RobloxClient::RobloxClient(
    const std::uint32_t processId,
    const Offsets::OffsetProvider& offsets)
    : m_session(std::make_shared<Session>()) {
    m_session->ProcessId = processId;
    m_session->Offsets = &offsets;
    m_session->FakeDataModelPointer = offsets.TryGet(
        Offsets::OffsetId::FakeDataModelPointer).value_or(0);
    m_session->FakeDataModelRealDataModel = offsets.TryGet(
        Offsets::OffsetId::FakeDataModelRealDataModel).value_or(0);
    m_session->DataModelPlaceId = offsets.TryGet(
        Offsets::OffsetId::DataModelPlaceId).value_or(0);
    m_session->DataModelGameId = offsets.TryGet(
        Offsets::OffsetId::DataModelGameId).value_or(0);
    m_session->DataModelCreatorId = offsets.TryGet(
        Offsets::OffsetId::DataModelCreatorId).value_or(0);
    m_session->DataModelGameLoaded = offsets.TryGet(
        Offsets::OffsetId::DataModelGameLoaded).value_or(0);
    m_session->VisualEnginePointer = offsets.TryGet(
        Offsets::OffsetId::VisualEnginePointer).value_or(0);
    m_session->VisualEngineDimensions = offsets.TryGet(
        Offsets::OffsetId::VisualEngineDimensions).value_or(0);
    m_session->VisualEngineViewMatrix = offsets.TryGet(
        Offsets::OffsetId::VisualEngineViewMatrix).value_or(0);
    m_session->InstanceNameContainer = offsets.TryGet(
        Offsets::OffsetId::InstanceNameContainer).value_or(0);
    m_session->InstanceName = offsets.TryGet(
        Offsets::OffsetId::InstanceName).value_or(0);
    m_session->InstanceClassDescriptor = offsets.TryGet(
        Offsets::OffsetId::InstanceClassDescriptor).value_or(0);
    m_session->InstanceParent = offsets.TryGet(
        Offsets::OffsetId::InstanceParent).value_or(0);
    m_session->InstanceChildrenStart = offsets.TryGet(
        Offsets::OffsetId::InstanceChildrenStart).value_or(0);
    m_session->InstanceChildrenEnd = offsets.TryGet(
        Offsets::OffsetId::InstanceChildrenEnd).value_or(0);
    m_session->InstanceClassName = offsets.TryGet(
        Offsets::OffsetId::InstanceClassName).value_or(0);
    m_session->InstanceClassBase = offsets.TryGet(
        Offsets::OffsetId::InstanceClassBase).value_or(0);
    m_session->PlayerUserId = offsets.TryGet(
        Offsets::OffsetId::PlayerUserId).value_or(0);
    m_session->PlayerDisplayName = offsets.TryGet(
        Offsets::OffsetId::PlayerDisplayName).value_or(0);
    m_session->PlayerAccountAge = offsets.TryGet(
        Offsets::OffsetId::PlayerAccountAge).value_or(0);
    m_session->PlayerModelInstance = offsets.TryGet(
        Offsets::OffsetId::PlayerModelInstance).value_or(0);
    m_session->BasePartPrimitive = offsets.TryGet(
        Offsets::OffsetId::BasePartPrimitive).value_or(0);
    m_session->PrimitiveAssemblyAngularVelocity = offsets.TryGet(
        Offsets::OffsetId::PrimitiveAssemblyAngularVelocity).value_or(0);
    m_session->PrimitiveAssemblyLinearVelocity = offsets.TryGet(
        Offsets::OffsetId::PrimitiveAssemblyLinearVelocity).value_or(0);
    m_session->PrimitiveFlags = offsets.TryGet(
        Offsets::OffsetId::PrimitiveFlags).value_or(0);
    m_session->PrimitivePosition = offsets.TryGet(
        Offsets::OffsetId::PrimitivePosition).value_or(0);
    m_session->PrimitiveRotation = offsets.TryGet(
        Offsets::OffsetId::PrimitiveRotation).value_or(0);
    m_session->PrimitiveSize = offsets.TryGet(
        Offsets::OffsetId::PrimitiveSize).value_or(0);
}

Core::Result<DataModel> RobloxClient::GetDataModel() const {
    if (!m_session) {
        return Core::Result<DataModel>::Failure("RobloxClient session is no longer available");
    }

    const auto location = Core::Process::FindLocation(m_session->ProcessId);
    if (!location.Succeeded()) {
        return Core::Result<DataModel>::Failure(
            "Unable to locate the target process: " + location.Message());
    }

    if (m_session->Offsets == nullptr) {
        return Core::Result<DataModel>::Failure(
            "RobloxClient offset provider is no longer available");
    }

    const auto resolvedAddress = ResolveDataModelAddress(
        m_session->ProcessId,
        *m_session->Offsets,
        location.Value());
    if (!resolvedAddress.Succeeded()) {
        return Core::Result<DataModel>::Failure(resolvedAddress.Message());
    }

    m_session->DataModelAddress = resolvedAddress.Value();
    return Core::Result<DataModel>::Success(
        DataModel(m_session, resolvedAddress.Value()));
}

Core::Result<Rendering::VisualEngine> RobloxClient::GetVisualEngine() const {
    if (!m_session) {
        return Core::Result<Rendering::VisualEngine>::Failure(
            "RobloxClient session is no longer available");
    }

    const auto location = Core::Process::FindLocation(m_session->ProcessId);
    if (!location.Succeeded()) {
        return Core::Result<Rendering::VisualEngine>::Failure(
            "Unable to locate the target process: " + location.Message());
    }

    if (m_session->Offsets == nullptr) {
        return Core::Result<Rendering::VisualEngine>::Failure(
            "RobloxClient offset provider is no longer available");
    }

    const auto resolvedAddress = ResolveVisualEngineAddress(
        m_session->ProcessId,
        *m_session->Offsets,
        location.Value());
    if (!resolvedAddress.Succeeded()) {
        return Core::Result<Rendering::VisualEngine>::Failure(
            resolvedAddress.Message());
    }

    return Core::Result<Rendering::VisualEngine>::Success(
        Rendering::VisualEngine(m_session, resolvedAddress.Value()));
}

std::uint32_t RobloxClient::ProcessId() const noexcept {
    return m_session ? m_session->ProcessId : 0;
}

}
