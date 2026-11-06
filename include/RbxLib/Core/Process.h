#pragma once

#include <RbxLib/Core/Result.h>

#include <cstdint>
#include <filesystem>

namespace RbxLib::Core {

class Process final {
public:
    [[nodiscard]] static Result<std::filesystem::path> FindLocation(
        std::uint32_t processId);
    [[nodiscard]] static Result<std::uintptr_t> FindModule(
        const std::filesystem::path& name,
        std::uint32_t processId);
};

}