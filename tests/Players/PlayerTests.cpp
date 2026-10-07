#include <RbxLib/Client/RobloxClient.h>
#include <RbxLib/Instances/Player.h>
#include <RbxLib/Offsets/OffsetProvider.h>

#include <Winux/Winux.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <optional>

namespace RbxLib::Roblox::Tests {
namespace {

class LivePlayerTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        const auto initialized = Winux::Platform::Initialize();
        ASSERT_TRUE(initialized.Succeeded()) << initialized.Message();
        m_initialized = true;

        m_process = &Winux::Get<Winux::Process>();
        m_memory = &Winux::Get<Winux::Memory>();

        const auto process = m_process->FindProcess(L"RobloxPlayerBeta.exe");
        ASSERT_TRUE(process.Succeeded()) << process.Message();
        ASSERT_TRUE(process.Value().has_value())
            << "RobloxPlayerBeta.exe must be running for live Player tests";
        m_processId = *process.Value();

        m_offsets = std::make_unique<Offsets::OffsetProvider>();
        const auto refresh = m_offsets->Refresh();
        ASSERT_TRUE(refresh.Success) << refresh.Error;
    }

    void TearDown() override {
        if (m_initialized) {
            Winux::Platform::Shutdown();
        }
    }

    std::unique_ptr<RobloxClient> MakeClient() const {
        return std::make_unique<RobloxClient>(
            m_processId, *m_process, *m_memory, *m_offsets);
    }

private:
    bool m_initialized = false;
    std::uint32_t m_processId = 0;
    Winux::Process* m_process = nullptr;
    Winux::Memory* m_memory = nullptr;
    std::unique_ptr<Offsets::OffsetProvider> m_offsets;
};

TEST_F(LivePlayerTestFixture, ReadsPlayerIdentityFields) {
    const auto client = MakeClient();
    const auto dataModel = client->GetDataModel();
    ASSERT_TRUE(dataModel.Succeeded()) << dataModel.Message();

    const auto playersService = dataModel.Value().FindFirstChildOfClass("Players");
    ASSERT_TRUE(playersService.Succeeded()) << playersService.Message();
    ASSERT_TRUE(playersService.Value().IsValid());

    const auto players = playersService.Value().GetChildren();
    ASSERT_TRUE(players.Succeeded()) << players.Message();
    ASSERT_FALSE(players.Value().empty());

    const auto player = Player::FromInstance(players.Value().front());
    ASSERT_TRUE(player.Succeeded()) << player.Message();

    const auto userId = player.Value().UserId();
    const auto displayName = player.Value().DisplayName();
    const auto accountAge = player.Value().AccountAge();

    ASSERT_TRUE(userId.Succeeded()) << userId.Message();
    ASSERT_TRUE(displayName.Succeeded()) << displayName.Message();
    ASSERT_TRUE(accountAge.Succeeded()) << accountAge.Message();
    EXPECT_NE(userId.Value(), 0u);
    EXPECT_FALSE(displayName.Value().empty());
    EXPECT_GE(accountAge.Value(), 0);
}

TEST_F(LivePlayerTestFixture, RejectsNonPlayerInstances) {
    const auto client = MakeClient();
    const auto dataModel = client->GetDataModel();
    ASSERT_TRUE(dataModel.Succeeded()) << dataModel.Message();

    const auto workspace = dataModel.Value().FindFirstChild("Workspace");
    ASSERT_TRUE(workspace.Succeeded()) << workspace.Message();
    ASSERT_TRUE(workspace.Value().IsValid());

    const auto player = Player::FromInstance(workspace.Value());
    EXPECT_FALSE(player.Succeeded());
    EXPECT_NE(player.Message().find("not a Player"), std::string::npos);
}

}
}