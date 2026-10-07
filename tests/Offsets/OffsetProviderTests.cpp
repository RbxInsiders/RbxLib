#include <RbxLib/Offsets/OffsetId.h>
#include <RbxLib/Offsets/OffsetProvider.h>
#include <RbxLib/Offsets/Sources/JsonSource.h>
#include <RbxLib/Offsets/Sources/ManualOffsetSource.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace RbxLib::Offsets::Tests {
namespace {

class FixedOffsetSource final : public Sources::OffsetSource {
public:
    explicit FixedOffsetSource(Sources::OffsetSourceResult result)
        : m_result(std::move(result)) {}

    [[nodiscard]] std::string_view Name() const noexcept override {
        return "Fixed test source";
    }

    [[nodiscard]] Sources::OffsetSourceResult Load() override {
        return m_result;
    }

    void SetResult(Sources::OffsetSourceResult result) {
        m_result = std::move(result);
    }

private:
    Sources::OffsetSourceResult m_result;
};

class FakeHttp {
public:
    std::vector<RbxLib::Network::HttpResponse> Responses;
    std::vector<std::string> Requests;

    RbxLib::Network::HttpResponse Get(std::string_view url) {
        Requests.emplace_back(url);
        if (m_nextResponse >= Responses.size()) {
            return {0, {}, "Unexpected HTTP request"};
        }
        return Responses[m_nextResponse++];
    }

private:
    std::size_t m_nextResponse = 0;
};

Sources::OffsetSourceResult MakeResult(
    std::string version,
    Sources::OffsetValues values) {
    return {std::move(version), std::move(values), {}, {}};
}

RbxLib::Network::HttpResponse SuccessResponse(std::string body) {
    return {200, std::move(body), {}};
}

std::string MakeOffsetsDocument(std::string_view version, std::string_view offsets) {
    return "{\"Roblox Version\":\"" + std::string(version)
        + "\",\"Offsets\":" + std::string(offsets) + "}";
}

TEST(OffsetProviderTests, ZeroValuesDoNotClaimAnOffset)
{
    OffsetProvider provider(std::make_unique<FixedOffsetSource>(MakeResult(
        "version-test",
        {{OffsetId::DataModelPlaceId, 0}})));
    provider.AddSource(std::make_unique<Sources::ManualOffsetSource>(
        "version-test",
        Sources::OffsetValues{{OffsetId::DataModelPlaceId, 416}}));

    const auto result = provider.Refresh();

    ASSERT_TRUE(result.Success) << result.Error;
    ASSERT_TRUE(provider.TryGet(OffsetId::DataModelPlaceId).has_value());
    EXPECT_EQ(*provider.TryGet(OffsetId::DataModelPlaceId), 416u);
}

TEST(OffsetProviderTests, DuplicateNonzeroValuesAreRejected)
{
    OffsetProvider provider(std::make_unique<FixedOffsetSource>(MakeResult(
        "version-test",
        {{OffsetId::DataModelPlaceId, 416}})));
    provider.AddSource(std::make_unique<Sources::ManualOffsetSource>(
        "version-test",
        Sources::OffsetValues{{OffsetId::DataModelPlaceId, 417}}));

    const auto result = provider.Refresh();

    EXPECT_FALSE(result.Success);
    EXPECT_NE(result.Error.find("DataModel.PlaceId"), std::string::npos);
    EXPECT_FALSE(provider.TryGet(OffsetId::DataModelPlaceId).has_value());
}

TEST(OffsetProviderTests, SourcesMustMatchPrimaryVersion)
{
    OffsetProvider provider(std::make_unique<FixedOffsetSource>(MakeResult(
        "version-live",
        {{OffsetId::FakeDataModelPointer, 123}})));
    provider.AddSource(std::make_unique<Sources::ManualOffsetSource>(
        "version-old",
        Sources::OffsetValues{{OffsetId::DataModelPlaceId, 416}}));

    const auto result = provider.Refresh();

    EXPECT_FALSE(result.Success);
    EXPECT_NE(result.Error.find("version does not match"), std::string::npos);
    EXPECT_FALSE(provider.TryGet(OffsetId::FakeDataModelPointer).has_value());
}

TEST(OffsetProviderTests, EnumNamesUseJsonGroupAndField)
{
    EXPECT_EQ(OffsetIdName(OffsetId::FakeDataModelRealDataModel),
        "FakeDataModel.RealDataModel");
    EXPECT_EQ(OffsetIdName(OffsetId::DataModelPlaceId), "DataModel.PlaceId");
    EXPECT_EQ(OffsetIdName(OffsetId::PlayerUserId), "Player.UserId");
}

TEST(JsonSourceTests, MapsKnownFieldsSkipsZeroAndReportsUnknownFields)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-test\r\n"),
        SuccessResponse(MakeOffsetsDocument("version-test",
            R"({"FakeDataModel":{"Pointer":112,"RealDataModel":0,"Extra":8},"DataModel":{"PlaceId":416,"GameLoaded":0,"Extra":1},"Other":{"Value":5}})"))
    };
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    const auto result = source.Load();

    ASSERT_TRUE(result.IsSuccess()) << result.Error;
    EXPECT_EQ(result.Version, "version-test");
    ASSERT_EQ(result.Values.size(), 2u);
    EXPECT_EQ(result.Values.at(OffsetId::FakeDataModelPointer), 112u);
    EXPECT_EQ(result.Values.at(OffsetId::DataModelPlaceId), 416u);
    EXPECT_FALSE(result.Values.contains(OffsetId::FakeDataModelRealDataModel));
    EXPECT_EQ(result.Diagnostics.size(), 3u);
    EXPECT_NE(std::find(result.Diagnostics.begin(), result.Diagnostics.end(),
        "Unknown offset field: FakeDataModel.Extra"), result.Diagnostics.end());
    EXPECT_NE(std::find(result.Diagnostics.begin(), result.Diagnostics.end(),
        "Unknown offset field: Other.Value"), result.Diagnostics.end());
}

TEST(JsonSourceTests, ReusesCachedDocumentWhenVersionIsUnchanged)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-test"),
        SuccessResponse(MakeOffsetsDocument("version-test",
            R"({"DataModel":{"PlaceId":416}})")),
        SuccessResponse("version-test")
    };
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    ASSERT_TRUE(source.Load().IsSuccess());
    const auto cachedResult = source.Load();

    ASSERT_TRUE(cachedResult.IsSuccess()) << cachedResult.Error;
    EXPECT_EQ(http.Requests.size(), 3u);
    EXPECT_EQ(http.Requests.back(), "https://offsets.imtheo.lol/roblox/version");
    EXPECT_EQ(cachedResult.Values.at(OffsetId::DataModelPlaceId), 416u);
}

TEST(JsonSourceTests, DownloadsNewDocumentWhenVersionChanges)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-one"),
        SuccessResponse(MakeOffsetsDocument("version-one",
            R"({"DataModel":{"PlaceId":416}})")),
        SuccessResponse("version-two"),
        SuccessResponse(MakeOffsetsDocument("version-two",
            R"({"DataModel":{"PlaceId":512}})"))
    };
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    ASSERT_TRUE(source.Load().IsSuccess());
    const auto updatedResult = source.Load();

    ASSERT_TRUE(updatedResult.IsSuccess()) << updatedResult.Error;
    EXPECT_EQ(updatedResult.Version, "version-two");
    EXPECT_EQ(updatedResult.Values.at(OffsetId::DataModelPlaceId), 512u);
    EXPECT_EQ(http.Requests.size(), 4u);
}

TEST(JsonSourceTests, RejectsDocumentForDifferentVersion)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-live"),
        SuccessResponse(MakeOffsetsDocument("version-old",
            R"({"DataModel":{"PlaceId":416}})"))
    };
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    const auto result = source.Load();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_NE(result.Error.find("does not match"), std::string::npos);
}

TEST(JsonSourceTests, RejectsMalformedJson)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-test"),
        SuccessResponse("{not json")
    };
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    const auto result = source.Load();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_NE(result.Error.find("Invalid offsets JSON"), std::string::npos);
}

TEST(JsonSourceTests, RejectsNonIntegerKnownOffset)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-test"),
        SuccessResponse(MakeOffsetsDocument("version-test",
            R"({"DataModel":{"PlaceId":416.5}})"))
    };
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    const auto result = source.Load();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_NE(result.Error.find("must be an integer"), std::string::npos);
}

TEST(JsonSourceTests, ReportsVersionEndpointFailure)
{
    FakeHttp http;
    http.Responses = {{503, "unavailable", {}}};
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    const auto result = source.Load();

    EXPECT_FALSE(result.IsSuccess());
    EXPECT_NE(result.Error.find("HTTP 503"), std::string::npos);
    EXPECT_EQ(http.Requests.size(), 1u);
}

TEST(OffsetProviderTests, SendsSourceDiagnosticsToHandler)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-test"),
        SuccessResponse(MakeOffsetsDocument("version-test",
            R"({"DataModel":{"PlaceId":416,"Unrecognized":8}})"))
    };
    std::vector<std::string> diagnostics;
    auto jsonSource = std::make_unique<Sources::JsonSource>(
        [&http](std::string_view url) { return http.Get(url); });
    OffsetProvider provider(std::move(jsonSource),
        [&diagnostics](std::string_view diagnostic) {
            diagnostics.emplace_back(diagnostic);
        });

    const auto result = provider.Refresh();

    ASSERT_TRUE(result.Success) << result.Error;
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_NE(diagnostics.front().find("DataModel.Unrecognized"), std::string::npos);
}

TEST(OffsetProviderTests, FailedSameVersionRefreshKeepsExistingSnapshot)
{
    OffsetProvider provider(std::make_unique<FixedOffsetSource>(MakeResult(
        "version-test",
        {{OffsetId::DataModelPlaceId, 416}})));

    ASSERT_TRUE(provider.Refresh().Success);
    provider.AddSource(std::make_unique<Sources::ManualOffsetSource>(
        "version-test",
        Sources::OffsetValues{{OffsetId::DataModelPlaceId, 417}}));

    const auto result = provider.Refresh();

    EXPECT_FALSE(result.Success);
    ASSERT_TRUE(provider.TryGet(OffsetId::DataModelPlaceId).has_value());
    EXPECT_EQ(*provider.TryGet(OffsetId::DataModelPlaceId), 416u);
}

TEST(OffsetProviderTests, VersionChangeInvalidatesSnapshotWhenAnotherSourceIsStale)
{
    auto primary = std::make_unique<FixedOffsetSource>(MakeResult(
        "version-one",
        {{OffsetId::FakeDataModelPointer, 112}}));
    auto* primaryPointer = primary.get();
    OffsetProvider provider(std::move(primary));
    provider.AddSource(std::make_unique<Sources::ManualOffsetSource>(
        "version-one",
        Sources::OffsetValues{{OffsetId::DataModelPlaceId, 416}}));

    ASSERT_TRUE(provider.Refresh().Success);
    primaryPointer->SetResult(MakeResult(
        "version-two",
        {{OffsetId::FakeDataModelPointer, 120}}));

    const auto result = provider.Refresh();

    EXPECT_FALSE(result.Success);
    EXPECT_FALSE(provider.TryGet(OffsetId::FakeDataModelPointer).has_value());
    EXPECT_TRUE(provider.CurrentVersion().empty());
}

TEST(JsonSourceTests, MapsPlayerIdentityFieldsAndSkipsZeroOffsets)
{
    FakeHttp http;
    http.Responses = {
        SuccessResponse("version-test"),
        SuccessResponse(MakeOffsetsDocument("version-test",
            R"({"Player":{"UserId":192,"DisplayName":296,"AccountAge":0}})"))
    };
    Sources::JsonSource source([&http](std::string_view url) {
        return http.Get(url);
    });

    const auto result = source.Load();

    ASSERT_TRUE(result.IsSuccess()) << result.Error;
    EXPECT_EQ(result.Values.at(OffsetId::PlayerUserId), 192u);
    EXPECT_EQ(result.Values.at(OffsetId::PlayerDisplayName), 296u);
    EXPECT_FALSE(result.Values.contains(OffsetId::PlayerAccountAge));
    EXPECT_TRUE(result.Diagnostics.empty());
}

}
}