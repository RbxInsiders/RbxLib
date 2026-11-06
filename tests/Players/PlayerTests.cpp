#include <RbxLib/Client/RobloxClient.h>
#include <RbxLib/Instances/BasePart.h>
#include <RbxLib/Instances/Player.h>
#include <RbxLib/Offsets/OffsetProvider.h>

#include <Winux/Winux.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
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
            m_processId, *m_offsets);
    }

private:
    bool m_initialized = false;
    std::uint32_t m_processId = 0;
    Winux::Process* m_process = nullptr;
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

TEST_F(LivePlayerTestFixture, ReadsCharacterBasePartPrimitiveFields) {
    const auto client = MakeClient();
    const auto dataModel = client->GetDataModel();
    ASSERT_TRUE(dataModel.Succeeded()) << dataModel.Message();

    const auto playersService = dataModel.Value().FindFirstChildOfClass("Players");
    ASSERT_TRUE(playersService.Succeeded()) << playersService.Message();
    const auto players = playersService.Value().GetChildren();
    ASSERT_TRUE(players.Succeeded()) << players.Message();
    ASSERT_FALSE(players.Value().empty());

    const auto player = Player::FromInstance(players.Value().front());
    ASSERT_TRUE(player.Succeeded()) << player.Message();
    const auto character = player.Value().Character();
    ASSERT_TRUE(character.Succeeded()) << character.Message();
    ASSERT_TRUE(character.Value().IsValid()) << "Player has no spawned character";

    const auto rootPart = character.Value().FindFirstChild("HumanoidRootPart");
    ASSERT_TRUE(rootPart.Succeeded()) << rootPart.Message();
    ASSERT_TRUE(rootPart.Value().IsValid());
    const auto basePart = BasePart::FromInstance(rootPart.Value());
    ASSERT_TRUE(basePart.Succeeded()) << basePart.Message();

    const auto primitive = basePart.Value().GetPrimitive();
    ASSERT_TRUE(primitive.Succeeded()) << primitive.Message();
    EXPECT_TRUE(primitive.Value().IsValid());

    const auto position = primitive.Value().Position();
    const auto rotation = primitive.Value().Rotation();
    const auto size = primitive.Value().Size();
    const auto linearVelocity = primitive.Value().AssemblyLinearVelocity();
    const auto angularVelocity = primitive.Value().AssemblyAngularVelocity();
    const auto flags = primitive.Value().Flags();
    const auto anchored = primitive.Value().IsAnchored();
    const auto canCollide = primitive.Value().CanCollide();
    const auto canTouch = primitive.Value().CanTouch();
    const auto canQuery = primitive.Value().CanQuery();

    ASSERT_TRUE(position.Succeeded()) << position.Message();
    ASSERT_TRUE(rotation.Succeeded()) << rotation.Message();
    ASSERT_TRUE(size.Succeeded()) << size.Message();
    ASSERT_TRUE(linearVelocity.Succeeded()) << linearVelocity.Message();
    ASSERT_TRUE(angularVelocity.Succeeded()) << angularVelocity.Message();
    ASSERT_TRUE(flags.Succeeded()) << flags.Message();
    ASSERT_TRUE(anchored.Succeeded()) << anchored.Message();
    ASSERT_TRUE(canCollide.Succeeded()) << canCollide.Message();
    ASSERT_TRUE(canTouch.Succeeded()) << canTouch.Message();
    ASSERT_TRUE(canQuery.Succeeded()) << canQuery.Message();

    const auto containsOnlyFiniteValues = [](const auto& values) {
        return std::all_of(values.begin(), values.end(), [](const float value) {
            return std::isfinite(value);
        });
    };
    EXPECT_TRUE(containsOnlyFiniteValues(position.Value()));
    EXPECT_TRUE(containsOnlyFiniteValues(rotation.Value()));
    EXPECT_TRUE(containsOnlyFiniteValues(size.Value()));
    EXPECT_TRUE(containsOnlyFiniteValues(linearVelocity.Value()));
    EXPECT_TRUE(containsOnlyFiniteValues(angularVelocity.Value()));
    EXPECT_GT(size.Value()[0], 0.0f);
    EXPECT_GT(size.Value()[1], 0.0f);
    EXPECT_GT(size.Value()[2], 0.0f);
}

}
}