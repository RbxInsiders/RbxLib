#pragma once

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

    [[nodiscard]] static Winux::Core::Result<BasePart> FromInstance(
        const Instance& instance);
    [[nodiscard]] Winux::Core::Result<Primitive> GetPrimitive() const;

private:
    explicit BasePart(const Instance& instance);
};

}