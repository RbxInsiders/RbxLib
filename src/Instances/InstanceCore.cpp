#include <RbxLib/Instances/Instance.h>

#include <RbxLib/Client/RobloxClient.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace RbxLib::Roblox {
namespace {

using Session = RobloxClient::Session;

std::shared_ptr<Session> GetSession(const std::shared_ptr<void>& session) {
    return std::static_pointer_cast<Session>(session);
}

template <typename TValue>
Winux::Core::Result<TValue> ReadValue(
    const std::shared_ptr<Session>& session,
    const std::uintptr_t address,
    std::string_view description) {
    if (!session || !session->Memory) {
        return Winux::Core::Result<TValue>::Failure(
            "Instance session is no longer available");
    }

    std::array<std::byte, sizeof(TValue)> bytes{};
    const auto read = session->Memory->ReadMemory(session->ProcessId, address, bytes);
    if (!read.Succeeded()) {
        return Winux::Core::Result<TValue>::Failure(
            "Unable to read " + std::string(description) + ": " + read.Message());
    }
    if (read.Value() != bytes.size()) {
        return Winux::Core::Result<TValue>::Failure(
            "Unable to read " + std::string(description) + ": short memory read");
    }

    TValue value{};
    std::memcpy(&value, bytes.data(), sizeof(value));
    return Winux::Core::Result<TValue>::Success(value);
}

Winux::Core::Result<std::string> ReadString(
    const std::shared_ptr<Session>& session,
    const std::uintptr_t address,
    std::string_view description) {
    const auto length = ReadValue<std::uint64_t>(
        session, address + 0x10, std::string(description) + " length");
    if (!length.Succeeded()) {
        return Winux::Core::Result<std::string>::Failure(length.Message());
    }
    if (length.Value() > 16ull * 1024ull * 1024ull) {
        return Winux::Core::Result<std::string>::Failure(
            std::string(description) + " length " + std::to_string(length.Value())
            + " exceeds the 16 MiB supported limit");
    }
    if (length.Value() == 0) {
        return Winux::Core::Result<std::string>::Success({});
    }

    auto stringAddress = address;
    if (length.Value() >= 16) {
        const auto pointer = ReadValue<std::uintptr_t>(
            session, address, std::string(description) + " pointer");
        if (!pointer.Succeeded()) {
            return Winux::Core::Result<std::string>::Failure(pointer.Message());
        }
        stringAddress = pointer.Value();
    }

    std::string value(static_cast<std::size_t>(length.Value()), '\0');
    const auto output = std::span<std::byte>(
        reinterpret_cast<std::byte*>(value.data()), value.size());
    const auto read = session->Memory->ReadMemory(
        session->ProcessId, stringAddress, output);
    if (!read.Succeeded()) {
        return Winux::Core::Result<std::string>::Failure(
            "Unable to read " + std::string(description) + ": " + read.Message());
    }
    if (read.Value() != output.size()) {
        return Winux::Core::Result<std::string>::Failure(
            "Unable to read " + std::string(description) + ": short memory read");
    }
    return Winux::Core::Result<std::string>::Success(std::move(value));
}

Winux::Core::Result<std::uintptr_t> ReadOffset(
    const std::shared_ptr<Session>& session,
    const std::uintptr_t base,
    const std::uintptr_t offset,
    std::string_view description) {
    if (offset == 0) {
        return Winux::Core::Result<std::uintptr_t>::Failure(
            "Offset is unavailable for " + std::string(description));
    }
    if (base > (std::numeric_limits<std::uintptr_t>::max)() - offset) {
        return Winux::Core::Result<std::uintptr_t>::Failure(
            "Address overflow while reading " + std::string(description));
    }
    return ReadValue<std::uintptr_t>(session, base + offset, description);
}

Winux::Core::Result<std::uintptr_t> FieldAddress(
    const std::uintptr_t base,
    const std::uintptr_t offset,
    std::string_view description) {
    if (offset == 0) {
        return Winux::Core::Result<std::uintptr_t>::Failure(
            "Offset is unavailable for " + std::string(description));
    }
    if (base > (std::numeric_limits<std::uintptr_t>::max)() - offset) {
        return Winux::Core::Result<std::uintptr_t>::Failure(
            "Address overflow while reading " + std::string(description));
    }
    return Winux::Core::Result<std::uintptr_t>::Success(base + offset);
}

}

Instance::Instance() = default;

Instance::Instance(std::shared_ptr<void> session, const std::uintptr_t address)
    : m_session(std::move(session)), m_address(address) {}

bool Instance::IsValid() const noexcept {
    return m_session != nullptr && m_address != 0;
}

std::shared_ptr<void> Instance::Session() const noexcept {
    return m_session;
}

std::uintptr_t Instance::Address() const noexcept {
    return m_address;
}

Winux::Core::Result<std::uint32_t> Instance::ReadUInt32Field(
    const std::uintptr_t offset,
    const std::string_view fieldName) const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<std::uint32_t>::Failure("Instance is not valid");
    }

    const auto address = FieldAddress(m_address, offset, fieldName);
    if (!address.Succeeded()) {
        return Winux::Core::Result<std::uint32_t>::Failure(address.Message());
    }
    return ReadValue<std::uint32_t>(session, address.Value(), fieldName);
}

Winux::Core::Result<std::int32_t> Instance::ReadInt32Field(
    const std::uintptr_t offset,
    const std::string_view fieldName) const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<std::int32_t>::Failure("Instance is not valid");
    }

    const auto address = FieldAddress(m_address, offset, fieldName);
    if (!address.Succeeded()) {
        return Winux::Core::Result<std::int32_t>::Failure(address.Message());
    }
    return ReadValue<std::int32_t>(session, address.Value(), fieldName);
}

Winux::Core::Result<std::string> Instance::ReadStringField(
    const std::uintptr_t offset,
    const std::string_view fieldName) const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<std::string>::Failure("Instance is not valid");
    }

    const auto address = FieldAddress(m_address, offset, fieldName);
    if (!address.Succeeded()) {
        return Winux::Core::Result<std::string>::Failure(address.Message());
    }
    return ReadString(session, address.Value(), fieldName);
}

Winux::Core::Result<std::string> Instance::Name() const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<std::string>::Failure("Instance is not valid");
    }

    const auto nameContainer = ReadOffset(
        session, m_address, session->InstanceNameContainer,
        "Instance.NameContainer");
    if (!nameContainer.Succeeded()) {
        return Winux::Core::Result<std::string>::Failure(nameContainer.Message());
    }
    if (nameContainer.Value() == 0) {
        return Winux::Core::Result<std::string>::Success({});
    }

    if (session->InstanceName == 0
        || nameContainer.Value() > (std::numeric_limits<std::uintptr_t>::max)()
            - session->InstanceName) {
        return Winux::Core::Result<std::string>::Failure(
            "Instance.Name address is invalid");
    }
    return ReadString(
        session,
        nameContainer.Value() + session->InstanceName,
        "Instance.Name");
}

Winux::Core::Result<std::string> Instance::ClassName() const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<std::string>::Failure("Instance is not valid");
    }

    const auto descriptor = ReadOffset(
        session, m_address, session->InstanceClassDescriptor,
        "Instance.ClassDescriptor");
    if (!descriptor.Succeeded()) {
        return Winux::Core::Result<std::string>::Failure(descriptor.Message());
    }
    if (descriptor.Value() == 0) {
        return Winux::Core::Result<std::string>::Failure(
            "Instance class descriptor is null");
    }

    const auto className = ReadOffset(
        session, descriptor.Value(), session->InstanceClassName,
        "Instance.ClassName");
    if (!className.Succeeded()) {
        return Winux::Core::Result<std::string>::Failure(className.Message());
    }
    if (className.Value() == 0) {
        return Winux::Core::Result<std::string>::Failure(
            "Instance class name pointer is null");
    }
    return ReadString(session, className.Value(), "Instance.ClassName");
}

Winux::Core::Result<Instance> Instance::Parent() const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<Instance>::Failure("Instance is not valid");
    }

    const auto parent = ReadOffset(
        session, m_address, session->InstanceParent, "Instance.Parent");
    if (!parent.Succeeded()) {
        return Winux::Core::Result<Instance>::Failure(parent.Message());
    }
    return Winux::Core::Result<Instance>::Success(Instance(m_session, parent.Value()));
}

Winux::Core::Result<std::vector<Instance>> Instance::GetChildren() const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<std::vector<Instance>>::Failure(
            "Instance is not valid");
    }

    const auto container = ReadOffset(
        session, m_address, session->InstanceChildrenStart,
        "Instance.ChildrenStart");
    if (!container.Succeeded()) {
        return Winux::Core::Result<std::vector<Instance>>::Failure(container.Message());
    }

    std::vector<Instance> children;
    if (container.Value() == 0) {
        return Winux::Core::Result<std::vector<Instance>>::Success(std::move(children));
    }

    const auto end = ReadOffset(
        session, container.Value(), session->InstanceChildrenEnd,
        "Instance.ChildrenEnd");
    if (!end.Succeeded()) {
        return Winux::Core::Result<std::vector<Instance>>::Failure(end.Message());
    }

    const auto begin = ReadValue<std::uintptr_t>(
        session, container.Value(), "Instance children begin");
    if (!begin.Succeeded()) {
        return Winux::Core::Result<std::vector<Instance>>::Failure(begin.Message());
    }
    if (begin.Value() == 0 || end.Value() == 0) {
        return Winux::Core::Result<std::vector<Instance>>::Success(std::move(children));
    }
    if (end.Value() < begin.Value() || (end.Value() - begin.Value()) % 0x10 != 0) {
        return Winux::Core::Result<std::vector<Instance>>::Failure(
            "Instance children range is invalid");
    }
    const auto childCount = (end.Value() - begin.Value()) / 0x10;
    if (childCount > 65536) {
        return Winux::Core::Result<std::vector<Instance>>::Failure(
            "Instance children range exceeds the supported size");
    }

    children.reserve(static_cast<std::size_t>(childCount));
    for (auto address = begin.Value(); address < end.Value(); address += 0x10) {
        const auto child = ReadValue<std::uintptr_t>(
            session, address, "Instance child pointer");
        if (!child.Succeeded()) {
            return Winux::Core::Result<std::vector<Instance>>::Failure(child.Message());
        }
        if (child.Value() != 0) {
            children.emplace_back(Instance(m_session, child.Value()));
        }
    }
    return Winux::Core::Result<std::vector<Instance>>::Success(std::move(children));
}

Winux::Core::Result<std::vector<Instance>> Instance::GetDescendants() const {
    const auto children = GetChildren();
    if (!children.Succeeded()) {
        return Winux::Core::Result<std::vector<Instance>>::Failure(children.Message());
    }

    std::vector<Instance> descendants;
    std::vector<Instance> pending(children.Value().rbegin(), children.Value().rend());
    std::unordered_set<std::uintptr_t> visited;
    visited.insert(m_address);

    while (!pending.empty()) {
        auto current = std::move(pending.back());
        pending.pop_back();
        if (!visited.insert(current.m_address).second) {
            continue;
        }
        if (descendants.size() >= 100000) {
            return Winux::Core::Result<std::vector<Instance>>::Failure(
                "Instance descendant count exceeds the supported size");
        }
        descendants.push_back(current);

        const auto nested = current.GetChildren();
        if (!nested.Succeeded()) {
            return Winux::Core::Result<std::vector<Instance>>::Failure(nested.Message());
        }
        for (auto child = nested.Value().rbegin(); child != nested.Value().rend(); ++child) {
            pending.push_back(*child);
        }
    }
    return Winux::Core::Result<std::vector<Instance>>::Success(std::move(descendants));
}

Winux::Core::Result<bool> Instance::IsA(const std::string_view className) const {
    const auto session = GetSession(m_session);
    if (!IsValid()) {
        return Winux::Core::Result<bool>::Failure("Instance is not valid");
    }

    const auto descriptor = ReadOffset(
        session, m_address, session->InstanceClassDescriptor,
        "Instance.ClassDescriptor");
    if (!descriptor.Succeeded()) {
        return Winux::Core::Result<bool>::Failure(descriptor.Message());
    }

    auto current = descriptor.Value();
    std::unordered_set<std::uintptr_t> visited;
    for (std::size_t depth = 0; current != 0 && depth < 256; ++depth) {
        if (!visited.insert(current).second) {
            return Winux::Core::Result<bool>::Failure(
                "Instance class descriptor chain contains a cycle");
        }
        const auto namePointer = ReadOffset(
            session, current, session->InstanceClassName, "Instance.ClassName");
        if (!namePointer.Succeeded()) {
            return Winux::Core::Result<bool>::Failure(namePointer.Message());
        }
        if (namePointer.Value() == 0) {
            return Winux::Core::Result<bool>::Success(false);
        }
        const auto name = ReadString(session, namePointer.Value(), "Instance.ClassName");
        if (!name.Succeeded()) {
            return Winux::Core::Result<bool>::Failure(name.Message());
        }
        if (name.Value() == className) {
            return Winux::Core::Result<bool>::Success(true);
        }
        if (name.Value() == "<<<ROOT>>>") {
            return Winux::Core::Result<bool>::Success(false);
        }

        const auto base = ReadOffset(
            session, current, session->InstanceClassBase, "Instance.ClassBase");
        if (!base.Succeeded()) {
            return Winux::Core::Result<bool>::Failure(base.Message());
        }
        current = base.Value();
    }
    if (current != 0) {
        return Winux::Core::Result<bool>::Failure(
            "Instance class descriptor chain exceeds the supported depth");
    }
    return Winux::Core::Result<bool>::Success(false);
}

}