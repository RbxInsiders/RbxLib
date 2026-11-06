#include <RbxLib/Client/RobloxClient.h>

#include <RbxLib/Offsets/OffsetProvider.h>
#include <RbxLib/Offsets/Sources/ManualOffsetSource.h>

#include <Winux/Winux.h>

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace RbxLib::Roblox::Tests {
namespace {

class LiveDataModelTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        const auto initialized = Winux::Platform::Initialize();
        if (!initialized.Succeeded()) {
            GTEST_SKIP() << "Unable to initialize Winux: " << initialized.Message();
        }

        m_process = &Winux::Get<Winux::Process>();
    }

    void TearDown() override {
        Winux::Platform::Shutdown();
    }

    void RequireRobloxProcess(std::optional<std::uint32_t>& processId) const {
        const auto result = m_process->FindProcess(L"RobloxPlayerBeta.exe");
        if (!result.Succeeded()) {
            GTEST_SKIP() << "Unable to find RobloxPlayerBeta.exe: " << result.Message();
        }

        processId = result.Value();
        if (!processId.has_value()) {
            GTEST_SKIP() << "Roblox is not running; open Roblox before running live tests.";
        }
    }

private:
    Winux::Process* m_process = nullptr;
};

TEST_F(LiveDataModelTestFixture, ResolvesDataModelAndVisualEngine) {
    std::optional<std::uint32_t> processId;
    RequireRobloxProcess(processId);
    ASSERT_TRUE(processId.has_value());

    auto provider = std::make_unique<RbxLib::Offsets::OffsetProvider>();
    const auto refresh = provider->Refresh();
    if (!refresh.Success) {
        GTEST_SKIP() << "Unable to load live Roblox offsets: " << refresh.Error;
    }

    RobloxClient client(*processId, *provider);
    const auto dataModel = client.GetDataModel();

    ASSERT_TRUE(dataModel.Succeeded()) << dataModel.Message();

    const auto placeId = dataModel.Value().PlaceId();
    const auto gameId = dataModel.Value().GameId();
    const auto creatorId = dataModel.Value().CreatorId();
    const auto gameLoaded = dataModel.Value().GameLoaded();
    const auto visualEngine = client.GetVisualEngine();

    ASSERT_TRUE(placeId.Succeeded());
    ASSERT_TRUE(gameId.Succeeded());
    ASSERT_TRUE(creatorId.Succeeded());
    ASSERT_TRUE(gameLoaded.Succeeded());
    ASSERT_TRUE(visualEngine.Succeeded()) << visualEngine.Message();

    const auto view = visualEngine.Value().Snapshot();
    ASSERT_TRUE(view.Succeeded()) << view.Message();

    EXPECT_GT(placeId.Value(), 0u);
    EXPECT_GT(gameId.Value(), 0u);
    EXPECT_GT(creatorId.Value(), 0u);
    EXPECT_TRUE(gameLoaded.Value());
    EXPECT_GT(view.Value().viewportSize[0], 0.0f);
    EXPECT_GT(view.Value().viewportSize[1], 0.0f);
}

TEST_F(LiveDataModelTestFixture, StaleDataModelFailsAfterClientIsDestroyed) {
    std::optional<std::uint32_t> processId;
    RequireRobloxProcess(processId);
    ASSERT_TRUE(processId.has_value());

    auto provider = std::make_unique<RbxLib::Offsets::OffsetProvider>();
    const auto refresh = provider->Refresh();
    if (!refresh.Success) {
        GTEST_SKIP() << "Unable to load live Roblox offsets: " << refresh.Error;
    }

    auto client = std::make_unique<RobloxClient>(
        *processId, *provider);
    const auto dataModel = client->GetDataModel();
    ASSERT_TRUE(dataModel.Succeeded()) << dataModel.Message();

    client.reset();
    EXPECT_TRUE(dataModel.Value().IsValid());
    EXPECT_TRUE(dataModel.Value().GameId().Succeeded());
}

}
}
