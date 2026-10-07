#include <RbxLib/Offsets/OffsetProvider.h>

#include <RbxLib/Offsets/Sources/JsonSource.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace RbxLib::Offsets {

OffsetProvider::OffsetProvider(DiagnosticHandler diagnosticHandler)
    : OffsetProvider(std::make_unique<Sources::JsonSource>(),
          std::move(diagnosticHandler)) {}

OffsetProvider::OffsetProvider(
    std::unique_ptr<Sources::OffsetSource> primarySource,
    DiagnosticHandler diagnosticHandler)
    : m_primarySource(std::move(primarySource)),
      m_diagnosticHandler(std::move(diagnosticHandler)) {
    if (!m_primarySource) {
        throw std::invalid_argument("OffsetProvider requires a primary source");
    }
}

void OffsetProvider::AddSource(std::unique_ptr<Sources::OffsetSource> source) {
    if (!source) {
        throw std::invalid_argument("OffsetProvider source must not be null");
    }

    std::unique_lock lock(m_snapshotMutex);
    m_sources.emplace_back(std::move(source));
}

OffsetRefreshResult OffsetProvider::Refresh() {
    std::lock_guard refreshLock(m_refreshMutex);

    auto primaryResult = m_primarySource->Load();
    EmitDiagnostics(m_primarySource->Name(), primaryResult.Diagnostics);
    if (!primaryResult.IsSuccess()) {
        std::unique_lock lock(m_snapshotMutex);
        m_values.clear();
        m_version.clear();
        m_snapshotReady = false;
        return {false, {}, primaryResult.Error.empty()
            ? "Primary offset source returned no version"
            : primaryResult.Error};
    }

    const std::string liveVersion = primaryResult.Version;
    std::vector<std::shared_ptr<Sources::OffsetSource>> sources;
    {
        std::unique_lock lock(m_snapshotMutex);
        if (m_version != liveVersion) {
            m_values.clear();
            m_snapshotReady = false;
        }
        sources = m_sources;
    }

    Sources::OffsetValues mergedValues;
    const auto mergeValues = [&mergedValues](
        const Sources::OffsetSourceResult& sourceResult,
        std::string_view sourceName) -> std::string {
        for (const auto& [id, value] : sourceResult.Values) {
            if (value == 0) {
                continue;
            }
            if (!mergedValues.emplace(id, value).second) {
                return "Duplicate nonzero value for "
                    + std::string(OffsetIdName(id)) + " from " + std::string(sourceName);
            }
        }
        return {};
    };

    if (auto error = mergeValues(primaryResult, m_primarySource->Name()); !error.empty()) {
        return {false, liveVersion, std::move(error)};
    }

    for (const auto& source : sources) {
        auto sourceResult = source->Load();
        EmitDiagnostics(source->Name(), sourceResult.Diagnostics);
        if (!sourceResult.IsSuccess()) {
            return {false, liveVersion, sourceResult.Error.empty()
                ? std::string(source->Name()) + " returned no version"
                : sourceResult.Error};
        }
        if (sourceResult.Version != liveVersion) {
            return {false, liveVersion, std::string(source->Name())
                + " version does not match the LIVE client version"};
        }

        if (auto error = mergeValues(sourceResult, source->Name()); !error.empty()) {
            return {false, liveVersion, std::move(error)};
        }
    }

    {
        std::unique_lock lock(m_snapshotMutex);
        m_values = std::move(mergedValues);
        m_version = liveVersion;
        m_snapshotReady = true;
    }

    return {true, liveVersion, {}};
}

std::optional<std::uintptr_t> OffsetProvider::TryGet(OffsetId id) const {
    std::shared_lock lock(m_snapshotMutex);
    if (!m_snapshotReady) {
        return std::nullopt;
    }

    const auto value = m_values.find(id);
    if (value == m_values.end()) {
        return std::nullopt;
    }
    return value->second;
}

std::string OffsetProvider::CurrentVersion() const {
    std::shared_lock lock(m_snapshotMutex);
    return m_snapshotReady ? m_version : std::string{};
}

void OffsetProvider::EmitDiagnostics(
    std::string_view sourceName,
    const std::vector<std::string>& diagnostics) const {
    if (!m_diagnosticHandler) {
        return;
    }

    for (const auto& diagnostic : diagnostics) {
        try {
            m_diagnosticHandler(std::string(sourceName) + ": " + diagnostic);
        } catch (...) {
        }
    }
}

}