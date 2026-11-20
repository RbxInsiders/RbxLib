#pragma once

#include <RbxLib/Core/Result.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace RbxLib::Core {

class Process final {
public:
    [[nodiscard]] static Result<void> Initialize();
    static void Shutdown() noexcept;
    [[nodiscard]] static Result<std::optional<std::uint32_t>> Find(
        std::wstring_view name);
    [[nodiscard]] static Result<std::filesystem::path> FindLocation(
        std::uint32_t processId);
    [[nodiscard]] static Result<std::uintptr_t> FindModule(
        const std::filesystem::path& name,
        std::uint32_t processId);
};

}