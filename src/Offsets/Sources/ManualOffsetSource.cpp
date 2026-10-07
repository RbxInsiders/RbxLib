#include <RbxLib/Offsets/Sources/ManualOffsetSource.h>

#include <utility>

namespace RbxLib::Offsets::Sources {

ManualOffsetSource::ManualOffsetSource(std::string version, OffsetValues values)
    : m_version(std::move(version)),
      m_values(std::move(values)) {}

std::string_view ManualOffsetSource::Name() const noexcept {
    return "Manual offsets";
}

OffsetSourceResult ManualOffsetSource::Load() {
    if (m_version.empty()) {
        return {{}, {}, {}, "Manual offset source requires a Roblox version"};
    }
    return {m_version, m_values, {}, {}};
}

}