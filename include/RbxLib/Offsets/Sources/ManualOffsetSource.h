#pragma once

#include <RbxLib/Offsets/Sources/OffsetSource.h>

#include <string>
#include <string_view>

namespace RbxLib::Offsets::Sources {

class ManualOffsetSource final : public OffsetSource {
public:
    ManualOffsetSource(std::string version, OffsetValues values);

    [[nodiscard]] std::string_view Name() const noexcept override;
    [[nodiscard]] OffsetSourceResult Load() override;

private:
    std::string m_version;
    OffsetValues m_values;
};

}