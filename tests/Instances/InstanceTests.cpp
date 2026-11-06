#include <RbxLib\Client\RobloxClient.h>

#include <RbxLib\Offsets\OffsetProvider.h>

#include <Winux\Winux.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <optional>

namespace RbxLib::Roblox::Tests {
namespace {

class LiveInstanceTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        const auto initialized = Winux::Platform::Initialize();
        ASSERT_TRUE(initialized.Succeeded()) << initialized.Message();
        m_initialized = true;

        m_process = &Winux::Get<Winux::Process>();

        const auto process = m_process->FindProcess(L"RobloxPlayerBeta.exe");
        ASSERT_TRUE(process.Succeeded()) << process.Message();
        ASSERT_TRUE(process.Value().has_value())
            << "RobloxPlayerBeta.exe must be running for live Instance tests";
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

TEST_F(LiveInstanceTestFixture, ReadsIdentityAndParent) {
    const auto client = MakeClient();
    const auto dataModel = client->GetDataModel();
    ASSERT_TRUE(dataModel.Succeeded()) << dataModel.Message();

    const auto name = dataModel.Value().Name();
    const auto className = dataModel.Value().ClassName();
    ASSERT_TRUE(name.Succeeded()) << name.Message();
    ASSERT_TRUE(className.Succeeded()) << className.Message();
    EXPECT_FALSE(name.Value().empty());
    EXPECT_EQ(className.Value(), "DataModel");

    const auto workspace = dataModel.Value().FindFirstChild("Workspace");
    ASSERT_TRUE(workspace.Succeeded()) << workspace.Message();
    ASSERT_TRUE(workspace.Value().IsValid());

    const auto parent = workspace.Value().Parent();
    ASSERT_TRUE(parent.Succeeded()) << parent.Message();
    ASSERT_TRUE(parent.Value().IsValid());
    const auto parentClassName = parent.Value().ClassName();
    ASSERT_TRUE(parentClassName.Succeeded()) << parentClassName.Message();
    EXPECT_EQ(parentClassName.Value(), "DataModel");
}

TEST_F(LiveInstanceTestFixture, TraversesChildrenAndMatchesClasses) {
    const auto client = MakeClient();
    const auto dataModel = client->GetDataModel();
    ASSERT_TRUE(dataModel.Succeeded()) << dataModel.Message();

    const auto children = dataModel.Value().GetChildren();
    ASSERT_TRUE(children.Succeeded()) << children.Message();
    ASSERT_FALSE(children.Value().empty());

    const auto workspaceByClass = dataModel.Value().FindFirstChildOfClass("Workspace");
    ASSERT_TRUE(workspaceByClass.Succeeded()) << workspaceByClass.Message();
    ASSERT_TRUE(workspaceByClass.Value().IsValid());

    const auto workspaceIsA = workspaceByClass.Value().IsA("Workspace");
    ASSERT_TRUE(workspaceIsA.Succeeded()) << workspaceIsA.Message();
    EXPECT_TRUE(workspaceIsA.Value());

    const auto workspaceByIsA = dataModel.Value().FindFirstChildWhichIsA("Workspace");
    ASSERT_TRUE(workspaceByIsA.Succeeded()) << workspaceByIsA.Message();
    EXPECT_TRUE(workspaceByIsA.Value().IsValid());

    const auto workspaceByPath = dataModel.Value().FindFirstChildFromPath("Workspace");
    ASSERT_TRUE(workspaceByPath.Succeeded()) << workspaceByPath.Message();
    EXPECT_TRUE(workspaceByPath.Value().IsValid());

    const auto workspaceDescendant = dataModel.Value().FindFirstDescendant("Workspace");
    ASSERT_TRUE(workspaceDescendant.Succeeded()) << workspaceDescendant.Message();
    EXPECT_TRUE(workspaceDescendant.Value().IsValid());

    const auto descendants = dataModel.Value().GetDescendants();
    ASSERT_TRUE(descendants.Succeeded()) << descendants.Message();
    EXPECT_FALSE(descendants.Value().empty());

    const auto missing = dataModel.Value().FindFirstChild("RbxLib_InstanceTest_Missing");
    ASSERT_TRUE(missing.Succeeded()) << missing.Message();
    EXPECT_FALSE(missing.Value().IsValid());
}

}
}