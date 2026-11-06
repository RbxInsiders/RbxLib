#include <RbxLib/Rendering/VisualEngine.h>

#include <RbxLib/Client/RobloxClient.h>
#include <RbxLib/Core/Memory.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace RbxLib::Rendering {
namespace {

template <typename TValue>
Core::Result<TValue> ReadVisualEngineField(
    const std::shared_ptr<void>& rawSession,
    const std::uintptr_t visualEngineAddress,
    const std::uintptr_t offset,
    const std::string_view fieldName) {
    if (!rawSession) {
        return Core::Result<TValue>::Failure(
            "VisualEngine session is no longer available");
    }
    if (visualEngineAddress == 0) {
        return Core::Result<TValue>::Failure("VisualEngine is not valid");
    }
    if (offset == 0) {
        return Core::Result<TValue>::Failure(
            "Offset is unavailable for " + std::string(fieldName));
    }
    if (visualEngineAddress > (std::numeric_limits<std::uintptr_t>::max)() - offset) {
        return Core::Result<TValue>::Failure(
            "Address overflow while reading " + std::string(fieldName));
    }

    const auto session = std::static_pointer_cast<Roblox::RobloxClient::Session>(rawSession);
    if (!session) {
        return Core::Result<TValue>::Failure(
            "VisualEngine session is no longer available");
    }

    std::array<std::byte, sizeof(TValue)> bytes{};
    const auto read = Core::Memory::Read(
        session->ProcessId, visualEngineAddress + offset, bytes);
    if (!read.Succeeded()) {
        return Core::Result<TValue>::Failure(
            "Unable to read " + std::string(fieldName) + ": " + read.Message());
    }
    if (read.Value() != bytes.size()) {
        return Core::Result<TValue>::Failure(
            "Unable to read " + std::string(fieldName) + ": short memory read");
    }

    TValue value{};
    std::memcpy(&value, bytes.data(), sizeof(value));
    return Core::Result<TValue>::Success(value);
}

}

VisualEngine::VisualEngine(std::shared_ptr<void> session, const std::uintptr_t address)
    : m_session(std::move(session)), m_address(address) {}

bool VisualEngine::IsValid() const noexcept {
    return m_session != nullptr && m_address != 0;
}

Core::Result<RenderView> VisualEngine::Snapshot() const {
    const auto session = std::static_pointer_cast<Roblox::RobloxClient::Session>(m_session);
    if (!session) {
        return Core::Result<RenderView>::Failure(
            "VisualEngine session is no longer available");
    }

    const auto matrix = ReadVisualEngineField<std::array<float, 16>>(
        m_session, m_address, session->VisualEngineViewMatrix, "VisualEngine.ViewMatrix");
    if (!matrix.Succeeded()) {
        return Core::Result<RenderView>::Failure(matrix.Message());
    }
    const auto dimensions = ReadVisualEngineField<std::array<float, 2>>(
        m_session, m_address, session->VisualEngineDimensions, "VisualEngine.Dimensions");
    if (!dimensions.Succeeded()) {
        return Core::Result<RenderView>::Failure(dimensions.Message());
    }

    const auto& viewport = dimensions.Value();
    if (!std::isfinite(viewport[0]) || !std::isfinite(viewport[1])
        || viewport[0] <= 0.0f || viewport[1] <= 0.0f) {
        return Core::Result<RenderView>::Failure(
            "VisualEngine returned invalid viewport dimensions");
    }
    if (!std::all_of(matrix.Value().begin(), matrix.Value().end(),
            [](const float value) { return std::isfinite(value); })) {
        return Core::Result<RenderView>::Failure(
            "VisualEngine returned a non-finite view matrix");
    }

    return Core::Result<RenderView>::Success(
        RenderView{matrix.Value(), viewport});
}

Core::Result<std::array<float, 2>> RenderView::WorldToScreen(
    const std::array<float, 3>& worldPosition) const {
    if (!std::isfinite(viewportSize[0]) || !std::isfinite(viewportSize[1])
        || viewportSize[0] <= 0.0f || viewportSize[1] <= 0.0f) {
        return Core::Result<std::array<float, 2>>::Failure(
            "RenderView has invalid viewport dimensions");
    }
    if (!std::all_of(viewMatrix.begin(), viewMatrix.end(),
            [](const float value) { return std::isfinite(value); })
        || !std::all_of(worldPosition.begin(), worldPosition.end(),
            [](const float value) { return std::isfinite(value); })) {
        return Core::Result<std::array<float, 2>>::Failure(
            "World-to-screen input contains a non-finite value");
    }

    const auto x = worldPosition[0];
    const auto y = worldPosition[1];
    const auto z = worldPosition[2];
    const auto clipX = viewMatrix[0] * x + viewMatrix[1] * y
        + viewMatrix[2] * z + viewMatrix[3];
    const auto clipY = viewMatrix[4] * x + viewMatrix[5] * y
        + viewMatrix[6] * z + viewMatrix[7];
    const auto clipW = viewMatrix[12] * x + viewMatrix[13] * y
        + viewMatrix[14] * z + viewMatrix[15];
    if (!std::isfinite(clipX) || !std::isfinite(clipY)
        || !std::isfinite(clipW) || clipW <= 1e-5f) {
        return Core::Result<std::array<float, 2>>::Failure(
            "World position is behind the camera or has invalid clip coordinates");
    }

    const auto screenX = (clipX / clipW + 1.0f) * 0.5f * viewportSize[0];
    const auto screenY = (1.0f - clipY / clipW) * 0.5f * viewportSize[1];
    if (!std::isfinite(screenX) || !std::isfinite(screenY)) {
        return Core::Result<std::array<float, 2>>::Failure(
            "World-to-screen projection produced non-finite coordinates");
    }

    return Core::Result<std::array<float, 2>>::Success({screenX, screenY});
}

}