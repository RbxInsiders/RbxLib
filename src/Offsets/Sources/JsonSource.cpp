#include <RbxLib/Offsets/Sources/JsonSource.h>

#include <nlohmann/json.hpp>

#include <array>
#include <cctype>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace RbxLib::Offsets::Sources {
namespace {

using Json = nlohmann::json;

struct JsonOffsetMapping {
    std::string_view Group;
    std::string_view Field;
    OffsetId Id;
};

constexpr std::array<JsonOffsetMapping, 25> Mappings{{
    {"FakeDataModel", "Pointer", OffsetId::FakeDataModelPointer},
    {"FakeDataModel", "RealDataModel", OffsetId::FakeDataModelRealDataModel},
    {"DataModel", "PlaceId", OffsetId::DataModelPlaceId},
    {"DataModel", "GameId", OffsetId::DataModelGameId},
    {"DataModel", "CreatorId", OffsetId::DataModelCreatorId},
    {"DataModel", "GameLoaded", OffsetId::DataModelGameLoaded},
    {"Instance", "NameContainer", OffsetId::InstanceNameContainer},
    {"Instance", "Name", OffsetId::InstanceName},
    {"Instance", "ClassDescriptor", OffsetId::InstanceClassDescriptor},
    {"Instance", "Parent", OffsetId::InstanceParent},
    {"Instance", "ChildrenStart", OffsetId::InstanceChildrenStart},
    {"Instance", "ChildrenEnd", OffsetId::InstanceChildrenEnd},
    {"Instance", "ClassName", OffsetId::InstanceClassName},
    {"Instance", "ClassBase", OffsetId::InstanceClassBase},
    {"Player", "UserId", OffsetId::PlayerUserId},
    {"Player", "DisplayName", OffsetId::PlayerDisplayName},
    {"Player", "AccountAge", OffsetId::PlayerAccountAge},
    {"Player", "ModelInstance", OffsetId::PlayerModelInstance},
    {"BasePart", "Primitive", OffsetId::BasePartPrimitive},
    {"Primitive", "AssemblyAngularVelocity", OffsetId::PrimitiveAssemblyAngularVelocity},
    {"Primitive", "AssemblyLinearVelocity", OffsetId::PrimitiveAssemblyLinearVelocity},
    {"Primitive", "Flags", OffsetId::PrimitiveFlags},
    {"Primitive", "Position", OffsetId::PrimitivePosition},
    {"Primitive", "Rotation", OffsetId::PrimitiveRotation},
    {"Primitive", "Size", OffsetId::PrimitiveSize},
}};

bool IsKnownGroup(std::string_view group) {
    return group == "FakeDataModel" || group == "DataModel"
        || group == "Instance" || group == "Player"
        || group == "BasePart" || group == "Primitive";
}

const JsonOffsetMapping* FindMapping(std::string_view group, std::string_view field) {
    for (const auto& mapping : Mappings) {
        if (mapping.Group == group && mapping.Field == field) {
            return &mapping;
        }
    }
    return nullptr;
}

std::string Trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

OffsetSourceResult Failure(std::string error) {
    OffsetSourceResult result;
    result.Error = std::move(error);
    return result;
}

std::string HttpError(std::string_view endpoint, const Network::HttpResponse& response) {
    if (!response.Error.empty()) {
        return std::string(endpoint) + ": " + response.Error;
    }
    return std::string(endpoint) + " returned HTTP " + std::to_string(response.StatusCode);
}

}

JsonSource::JsonSource()
    : m_get([this](std::string_view url) {
        return m_httpClient.Get(url);
    }) {}

JsonSource::JsonSource(GetFunction get)
    : m_get(std::move(get)) {
    if (!m_get) {
        throw std::invalid_argument("JsonSource GET function must not be empty");
    }
}

std::string_view JsonSource::Name() const noexcept {
    return "Offsets JSON service";
}

OffsetSourceResult JsonSource::Load() {
    const auto versionResponse = m_get(
        "https://offsets.imtheo.lol/roblox/version");
    if (!versionResponse.IsSuccess()) {
        return Failure(HttpError("GET /roblox/version", versionResponse));
    }

    const std::string liveVersion = Trim(versionResponse.Body);
    if (liveVersion.empty()) {
        return Failure("GET /roblox/version returned an empty version");
    }

    if (m_cachedVersion == liveVersion) {
        return {m_cachedVersion, m_cachedValues, m_cachedDiagnostics, {}};
    }

    const auto offsetsResponse = m_get(
        "https://offsets.imtheo.lol/offsets.json");
    if (!offsetsResponse.IsSuccess()) {
        return Failure(HttpError("GET /offsets.json", offsetsResponse));
    }

    auto result = ParseDocument(liveVersion, offsetsResponse.Body);
    if (result.IsSuccess()) {
        m_cachedVersion = result.Version;
        m_cachedValues = result.Values;
        m_cachedDiagnostics = result.Diagnostics;
    }
    return result;
}

OffsetSourceResult JsonSource::ParseDocument(
    std::string_view expectedVersion,
    std::string_view document) const {
    Json root;
    try {
        root = Json::parse(document);
    } catch (const Json::exception& exception) {
        return Failure(std::string("Invalid offsets JSON: ") + exception.what());
    }

    if (!root.is_object()) {
        return Failure("Offsets JSON root must be an object");
    }

    const auto version = root.find("Roblox Version");
    if (version == root.end() || !version->is_string()) {
        return Failure("Offsets JSON is missing string metadata 'Roblox Version'");
    }

    const std::string documentVersion = version->get<std::string>();
    if (documentVersion != expectedVersion) {
        return Failure("Offsets JSON version does not match the LIVE client version");
    }

    const auto offsets = root.find("Offsets");
    if (offsets == root.end() || !offsets->is_object()) {
        return Failure("Offsets JSON is missing object 'Offsets'");
    }

    OffsetSourceResult result;
    result.Version = documentVersion;

    for (auto group = offsets->begin(); group != offsets->end(); ++group) {
        if (!group.value().is_object()) {
            if (IsKnownGroup(group.key())) {
                return Failure("Offset group '" + group.key() + "' must be an object");
            }
            result.Diagnostics.push_back("Unknown offset group: " + group.key());
            continue;
        }

        for (auto field = group.value().begin(); field != group.value().end(); ++field) {
            const auto* mapping = FindMapping(group.key(), field.key());
            if (mapping == nullptr) {
                result.Diagnostics.push_back(
                    "Unknown offset field: " + group.key() + "." + field.key());
                continue;
            }

            if (!field.value().is_number_integer()) {
                return Failure("Offset '" + group.key() + "." + field.key()
                    + "' must be an integer");
            }

            std::uint64_t value = 0;
            if (field.value().is_number_unsigned()) {
                value = field.value().get<std::uint64_t>();
            } else {
                const auto signedValue = field.value().get<std::int64_t>();
                if (signedValue < 0) {
                    return Failure("Offset '" + group.key() + "." + field.key()
                        + "' cannot be negative");
                }
                value = static_cast<std::uint64_t>(signedValue);
            }

            if (value > std::numeric_limits<std::uintptr_t>::max()) {
                return Failure("Offset '" + group.key() + "." + field.key()
                    + "' exceeds the address-sized integer range");
            }

            if (value != 0) {
                result.Values.emplace(mapping->Id, static_cast<std::uintptr_t>(value));
            }
        }
    }

    return result;
}

}