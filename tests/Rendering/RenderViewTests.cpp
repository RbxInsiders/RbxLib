#include <RbxLib/Rendering/RenderView.h>

#include <gtest/gtest.h>

#include <array>
#include <limits>

namespace RbxLib::Rendering::Tests {
namespace {

RenderView IdentityView() {
    RenderView view;
    view.viewMatrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    view.viewportSize = {800.0f, 600.0f};
    return view;
}

TEST(RenderViewTests, ProjectsWorldOriginToViewportCenter) {
    const auto view = IdentityView();

    const auto screen = view.WorldToScreen({0.0f, 0.0f, 0.0f});

    ASSERT_TRUE(screen.Succeeded()) << screen.Message();
    EXPECT_FLOAT_EQ(screen.Value()[0], 400.0f);
    EXPECT_FLOAT_EQ(screen.Value()[1], 300.0f);
}

TEST(RenderViewTests, MapsPositiveClipEdgesToTopRight) {
    const auto view = IdentityView();

    const auto screen = view.WorldToScreen({1.0f, 1.0f, 0.0f});

    ASSERT_TRUE(screen.Succeeded()) << screen.Message();
    EXPECT_FLOAT_EQ(screen.Value()[0], 800.0f);
    EXPECT_FLOAT_EQ(screen.Value()[1], 0.0f);
}

TEST(RenderViewTests, RejectsPositionsBehindCamera) {
    auto view = IdentityView();
    view.viewMatrix[15] = -1.0f;

    const auto screen = view.WorldToScreen({0.0f, 0.0f, 0.0f});

    EXPECT_FALSE(screen.Succeeded());
}

TEST(RenderViewTests, RejectsInvalidViewportAndNonFiniteInput) {
    auto view = IdentityView();
    view.viewportSize[0] = 0.0f;
    EXPECT_FALSE(view.WorldToScreen({0.0f, 0.0f, 0.0f}).Succeeded());

    view = IdentityView();
    EXPECT_FALSE(view.WorldToScreen({
        std::numeric_limits<float>::infinity(), 0.0f, 0.0f}).Succeeded());
}

}
}