#pragma once

#include <RbxLib/Core/Result.h>

#include <cstddef>
#include <cstdint>
#include <span>

namespace RbxLib::Core {

class Memory final {
public:
    [[nodiscard]] static Result<std::size_t> Read(
        std::uint32_t processId,
        std::uintptr_t remoteAddress,
        std::span<std::byte> output);
};

}