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
    InstanceNameContainer,
    InstanceName,
    InstanceClassDescriptor,
    InstanceParent,
    InstanceChildrenStart,
    InstanceChildrenEnd,
    InstanceClassName,
    InstanceClassBase,
};

[[nodiscard]] constexpr std::string_view OffsetIdName(OffsetId id) noexcept {
    switch (id) {
    case OffsetId::FakeDataModelPointer: return "FakeDataModel.Pointer";
    case OffsetId::FakeDataModelRealDataModel: return "FakeDataModel.RealDataModel";
    case OffsetId::DataModelPlaceId: return "DataModel.PlaceId";
    case OffsetId::DataModelGameId: return "DataModel.GameId";
    case OffsetId::DataModelCreatorId: return "DataModel.CreatorId";
    case OffsetId::DataModelGameLoaded: return "DataModel.GameLoaded";
    case OffsetId::InstanceNameContainer: return "Instance.NameContainer";
    case OffsetId::InstanceName: return "Instance.Name";
    case OffsetId::InstanceClassDescriptor: return "Instance.ClassDescriptor";
    case OffsetId::InstanceParent: return "Instance.Parent";
    case OffsetId::InstanceChildrenStart: return "Instance.ChildrenStart";
    case OffsetId::InstanceChildrenEnd: return "Instance.ChildrenEnd";
    case OffsetId::InstanceClassName: return "Instance.ClassName";
    case OffsetId::InstanceClassBase: return "Instance.ClassBase";
    }

    return "UnknownOffset";
}

}