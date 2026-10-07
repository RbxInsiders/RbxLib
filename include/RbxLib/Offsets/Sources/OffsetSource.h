#pragma once

#include <RbxLib/Offsets/OffsetId.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace RbxLib::Offsets::Sources {

using OffsetValues = std::unordered_map<OffsetId, std::uintptr_t>;

struct OffsetSourceResult {
    std::string Version;
    OffsetValues Values;
    std::vector<std::string> Diagnostics;
    std::string Error;

    [[nodiscard]] bool IsSuccess() const noexcept {
        return Error.empty() && !Version.empty();
    }
};

class OffsetSource {
public:
    virtual ~OffsetSource() = default;
    [[nodiscard]] virtual std::string_view Name() const noexcept = 0;
    [[nodiscard]] virtual OffsetSourceResult Load() = 0;
};

}