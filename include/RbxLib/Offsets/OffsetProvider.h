#pragma once

#include <RbxLib/Offsets/Sources/OffsetSource.h>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

namespace RbxLib::Offsets {

struct OffsetRefreshResult {
    bool Success = false;
    std::string Version;
    std::string Error;
};

class OffsetProvider final {
public:
    using DiagnosticHandler = std::function<void(std::string_view)>;

    explicit OffsetProvider(DiagnosticHandler diagnosticHandler = {});
    explicit OffsetProvider(
        std::unique_ptr<Sources::OffsetSource> primarySource,
        DiagnosticHandler diagnosticHandler = {});

    void AddSource(std::unique_ptr<Sources::OffsetSource> source);
    [[nodiscard]] OffsetRefreshResult Refresh();
    [[nodiscard]] std::optional<std::uintptr_t> TryGet(OffsetId id) const;
    [[nodiscard]] std::string CurrentVersion() const;

private:
    void EmitDiagnostics(
        std::string_view sourceName,
        const std::vector<std::string>& diagnostics) const;

    std::unique_ptr<Sources::OffsetSource> m_primarySource;
    std::vector<std::shared_ptr<Sources::OffsetSource>> m_sources;
    DiagnosticHandler m_diagnosticHandler;
    mutable std::shared_mutex m_snapshotMutex;
    std::mutex m_refreshMutex;
    Sources::OffsetValues m_values;
    std::string m_version;
    bool m_snapshotReady = false;
};

}