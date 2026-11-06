#pragma once

#include <RbxLib/Core/Result.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace RbxLib::Roblox {

class RobloxClient;

class Instance {
public:
    Instance();
    Instance(const Instance& other) = default;
    Instance(Instance&& other) noexcept = default;
    Instance& operator=(const Instance& other) = default;
    Instance& operator=(Instance&& other) noexcept = default;

    [[nodiscard]] bool IsValid() const noexcept;
    [[nodiscard]] Core::Result<std::string> Name() const;
    [[nodiscard]] Core::Result<std::string> ClassName() const;
    [[nodiscard]] Core::Result<Instance> Parent() const;
    [[nodiscard]] Core::Result<std::vector<Instance>> GetChildren() const;
    [[nodiscard]] Core::Result<std::vector<Instance>> GetDescendants() const;
    [[nodiscard]] Core::Result<Instance> FindFirstChild(std::string_view name) const;
    [[nodiscard]] Core::Result<Instance> FindFirstChildFromPath(std::string_view path) const;
    [[nodiscard]] Core::Result<Instance> FindFirstDescendant(std::string_view name) const;
    [[nodiscard]] Core::Result<Instance> FindFirstChildOfClass(std::string_view className) const;
    [[nodiscard]] Core::Result<Instance> FindFirstChildWhichIsA(std::string_view className) const;
    [[nodiscard]] Core::Result<bool> IsA(std::string_view className) const;

protected:
    explicit Instance(std::shared_ptr<void> session, std::uintptr_t address);

    [[nodiscard]] std::shared_ptr<void> Session() const noexcept;
    [[nodiscard]] std::uintptr_t Address() const noexcept;
    [[nodiscard]] Instance AtAddress(std::uintptr_t address) const;
    [[nodiscard]] Core::Result<std::uintptr_t> ReadPointerField(
        std::uintptr_t offset, std::string_view fieldName) const;
    [[nodiscard]] Core::Result<std::uint32_t> ReadUInt32Field(
        std::uintptr_t offset, std::string_view fieldName) const;
    [[nodiscard]] Core::Result<std::int32_t> ReadInt32Field(
        std::uintptr_t offset, std::string_view fieldName) const;
    [[nodiscard]] Core::Result<std::string> ReadStringField(
        std::uintptr_t offset, std::string_view fieldName) const;

private:
    std::shared_ptr<void> m_session;
    std::uintptr_t m_address = 0;
};

}
