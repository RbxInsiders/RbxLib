#pragma once

#include <RbxLib/Core/Result.h>

#include <array>

namespace RbxLib::Rendering {

struct RenderView final {
    std::array<float, 16> viewMatrix{};
    std::array<float, 2> viewportSize{};

    [[nodiscard]] Core::Result<std::array<float, 2>> WorldToScreen(
        const std::array<float, 3>& worldPosition) const;
};

}