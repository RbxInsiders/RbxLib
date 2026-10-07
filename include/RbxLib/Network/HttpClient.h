#pragma once

#include <string>
#include <string_view>

namespace RbxLib::Network {

struct HttpResponse {
    long StatusCode = 0;
    std::string Body;
    std::string Error;

    [[nodiscard]] bool IsSuccess() const noexcept {
        return Error.empty() && StatusCode >= 200 && StatusCode < 300;
    }
};

class HttpClient final {
public:
    [[nodiscard]] HttpResponse Get(std::string_view url) const;
};

}