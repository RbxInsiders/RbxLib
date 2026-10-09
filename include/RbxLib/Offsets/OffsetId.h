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
    PlayerUserId,
    PlayerDisplayName,
    PlayerAccountAge,
    PlayerModelInstance,
    BasePartPrimitive,
    PrimitiveAssemblyAngularVelocity,
    PrimitiveAssemblyLinearVelocity,
    PrimitiveFlags,
    PrimitivePosition,
    PrimitiveRotation,
    PrimitiveSize,
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
    case OffsetId::PlayerUserId: return "Player.UserId";
    case OffsetId::PlayerDisplayName: return "Player.DisplayName";
    case OffsetId::PlayerAccountAge: return "Player.AccountAge";
    case OffsetId::PlayerModelInstance: return "Player.ModelInstance";
    case OffsetId::BasePartPrimitive: return "BasePart.Primitive";
    case OffsetId::PrimitiveAssemblyAngularVelocity: return "Primitive.AssemblyAngularVelocity";
    case OffsetId::PrimitiveAssemblyLinearVelocity: return "Primitive.AssemblyLinearVelocity";
    case OffsetId::PrimitiveFlags: return "Primitive.Flags";
    case OffsetId::PrimitivePosition: return "Primitive.Position";
    case OffsetId::PrimitiveRotation: return "Primitive.Rotation";
    case OffsetId::PrimitiveSize: return "Primitive.Size";
    }

    return "UnknownOffset";
}

}