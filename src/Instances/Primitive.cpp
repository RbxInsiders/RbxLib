#include <RbxLib/Instances/Primitive.h>

#include <RbxLib/Client/RobloxClient.h>
#include <RbxLib/Core/Memory.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace RbxLib::Roblox {
namespace {

template <typename TValue>
Core::Result<TValue> ReadPrimitiveField(
    const std::shared_ptr<void>& rawSession,
    const std::uintptr_t primitiveAddress,
    const std::uintptr_t offset,
    const std::string_view fieldName) {
    if (!rawSession) {
        return Core::Result<TValue>::Failure(
            "Primitive session is no longer available");
    }
    if (primitiveAddress == 0) {
        return Core::Result<TValue>::Failure("Primitive is not valid");
    }
    if (offset == 0) {
        return Core::Result<TValue>::Failure(
            "Offset is unavailable for " + std::string(fieldName));
    }
    if (primitiveAddress > (std::numeric_limits<std::uintptr_t>::max)() - offset) {
        return Core::Result<TValue>::Failure(
            "Address overflow while reading " + std::string(fieldName));
    }

    const auto session = std::static_pointer_cast<RobloxClient::Session>(rawSession);
    if (!session) {
        return Core::Result<TValue>::Failure(
            "Primitive session is no longer available");
    }

    std::array<std::byte, sizeof(TValue)> bytes{};
    const auto read = Core::Memory::Read(
        session->ProcessId, primitiveAddress + offset, bytes);
    if (!read.Succeeded()) {
        return Core::Result<TValue>::Failure(
            "Unable to read " + std::string(fieldName) + ": " + read.Message());
    }
    if (read.Value() != bytes.size()) {
        return Core::Result<TValue>::Failure(
            "Unable to read " + std::string(fieldName) + ": short memory read");
    }

    TValue value{};
    std::memcpy(&value, bytes.data(), sizeof(value));
    return Core::Result<TValue>::Success(value);
}

Core::Result<bool> HasFlag(
    const Primitive& primitive,
    const std::uint32_t mask) {
    const auto flags = primitive.Flags();
    if (!flags.Succeeded()) {
        return Core::Result<bool>::Failure(flags.Message());
    }
    return Core::Result<bool>::Success((flags.Value() & mask) != 0);
}

}

Primitive::Primitive(std::shared_ptr<void> session, const std::uintptr_t address)
    : m_session(std::move(session)), m_address(address) {}

bool Primitive::IsValid() const noexcept {
    return m_session != nullptr && m_address != 0;
}

Core::Result<std::array<float, 3>> Primitive::Position() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(m_session);
    if (!session) {
        return Core::Result<std::array<float, 3>>::Failure(
            "Primitive session is unavailable");
    }
    return ReadPrimitiveField<std::array<float, 3>>(
        m_session, m_address, session->PrimitivePosition, "Primitive.Position");
}

Core::Result<std::array<float, 9>> Primitive::Rotation() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(m_session);
    if (!session) {
        return Core::Result<std::array<float, 9>>::Failure(
            "Primitive session is unavailable");
    }
    return ReadPrimitiveField<std::array<float, 9>>(
        m_session, m_address, session->PrimitiveRotation, "Primitive.Rotation");
}

Core::Result<std::array<float, 3>> Primitive::Size() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(m_session);
    if (!session) {
        return Core::Result<std::array<float, 3>>::Failure(
            "Primitive session is unavailable");
    }
    return ReadPrimitiveField<std::array<float, 3>>(
        m_session, m_address, session->PrimitiveSize, "Primitive.Size");
}

Core::Result<std::array<float, 3>> Primitive::AssemblyLinearVelocity() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(m_session);
    if (!session) {
        return Core::Result<std::array<float, 3>>::Failure(
            "Primitive session is unavailable");
    }
    return ReadPrimitiveField<std::array<float, 3>>(
        m_session, m_address, session->PrimitiveAssemblyLinearVelocity,
        "Primitive.AssemblyLinearVelocity");
}

Core::Result<std::array<float, 3>> Primitive::AssemblyAngularVelocity() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(m_session);
    if (!session) {
        return Core::Result<std::array<float, 3>>::Failure(
            "Primitive session is unavailable");
    }
    return ReadPrimitiveField<std::array<float, 3>>(
        m_session, m_address, session->PrimitiveAssemblyAngularVelocity,
        "Primitive.AssemblyAngularVelocity");
}

Core::Result<std::uint32_t> Primitive::Flags() const {
    const auto session = std::static_pointer_cast<RobloxClient::Session>(m_session);
    if (!session) {
        return Core::Result<std::uint32_t>::Failure(
            "Primitive session is unavailable");
    }
    return ReadPrimitiveField<std::uint32_t>(
        m_session, m_address, session->PrimitiveFlags, "Primitive.Flags");
}

Core::Result<bool> Primitive::IsAnchored() const {
    return HasFlag(*this, 0x2);
}

Core::Result<bool> Primitive::CanCollide() const {
    return HasFlag(*this, 0x8);
}

Core::Result<bool> Primitive::CanTouch() const {
    return HasFlag(*this, 0x10);
}

Core::Result<bool> Primitive::CanQuery() const {
    return HasFlag(*this, 0x20);
}

}