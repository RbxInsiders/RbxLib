#pragma once

#include <RbxLib/Instances/Instance.h>

#include <Winux/Winux.h>

#include <cstdint>
#include <memory>

namespace RbxLib::Roblox {

class DataModel final : public Instance {
public:
    DataModel();
    DataModel(const DataModel& other) = default;
    DataModel(DataModel&& other) noexcept = default;
    DataModel& operator=(const DataModel& other) = default;
    DataModel& operator=(DataModel&& other) noexcept = default;

    [[nodiscard]] Winux::Core::Result<std::uint64_t> PlaceId() const;
    [[nodiscard]] Winux::Core::Result<std::uint64_t> GameId() const;
    [[nodiscard]] Winux::Core::Result<std::uint64_t> CreatorId() const;
    [[nodiscard]] Winux::Core::Result<bool> GameLoaded() const;

private:
    friend class RobloxClient;

    explicit DataModel(std::shared_ptr<void> session, std::uintptr_t address);
};

}
