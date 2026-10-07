#pragma once

#include <cstdint>
#include <string_view>

namespace RbxLib::Offsets {

enum class OffsetId : std::uint16_t {
    FakeDataModelPointer,
    FakeDataModelRealDataModel,
    DataModelPlaceId,
    DataModelGameId,
    DataModelCreatorId,
    DataModelGameLoaded,
};

[[nodiscard]] constexpr std::string_view OffsetIdName(OffsetId id) noexcept {
    switch (id) {
    case OffsetId::FakeDataModelPointer: return "FakeDataModel.Pointer";
    case OffsetId::FakeDataModelRealDataModel: return "FakeDataModel.RealDataModel";
    case OffsetId::DataModelPlaceId: return "DataModel.PlaceId";
    case OffsetId::DataModelGameId: return "DataModel.GameId";
    case OffsetId::DataModelCreatorId: return "DataModel.CreatorId";
    case OffsetId::DataModelGameLoaded: return "DataModel.GameLoaded";
    }

    return "UnknownOffset";
}

}