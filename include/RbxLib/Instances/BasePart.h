#pragma once

#include <RbxLib/Core/Result.h>
#include <RbxLib/Instances/Instance.h>
#include <RbxLib/Instances/Primitive.h>

#include <cstdint>

namespace RbxLib::Roblox {

class BasePart final : public Instance {
public:
    BasePart() = default;
    BasePart(const BasePart& other) = default;
    BasePart(BasePart&& other) noexcept = default;
    BasePart& operator=(const BasePart& other) = default;
    BasePart& operator=(BasePart&& other) noexcept = default;

    [[nodiscard]] static Core::Result<BasePart> FromInstance(
        const Instance& instance);
    [[nodiscard]] Core::Result<Primitive> GetPrimitive() const;

private:
    explicit BasePart(const Instance& instance);
};

}