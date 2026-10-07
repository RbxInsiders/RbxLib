#pragma once

#include <RbxLib/Network/HttpClient.h>
#include <RbxLib/Offsets/Sources/OffsetSource.h>

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace RbxLib::Offsets::Sources {

class JsonSource final : public OffsetSource {
public:
    using GetFunction = std::function<Network::HttpResponse(std::string_view)>;

    JsonSource();
    explicit JsonSource(GetFunction get);

    [[nodiscard]] std::string_view Name() const noexcept override;
    [[nodiscard]] OffsetSourceResult Load() override;

private:
    [[nodiscard]] OffsetSourceResult ParseDocument(
        std::string_view expectedVersion,
        std::string_view document) const;

    Network::HttpClient m_httpClient;
    GetFunction m_get;
    std::string m_cachedVersion;
    OffsetValues m_cachedValues;
    std::vector<std::string> m_cachedDiagnostics;
};

}