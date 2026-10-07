#include <RbxLib/Network/HttpClient.h>

#include <Winux/Winux.h>

#include <chrono>
#include <cstddef>
#include <string>
#include <utility>

namespace RbxLib::Network {

HttpResponse HttpClient::Get(std::string_view url) const {
    HttpResponse response;
    if (url.empty()) {
        response.Error = "URL must not be empty";
        return response;
    }

    const auto initialized = Winux::Platform::IsInitialized();
    if (initialized.Failed()) {
        response.Error = initialized.Message();
        return response;
    }
    if (!initialized.Value()) {
        response.Error = "Winux platform must be initialized before using HttpClient";
        return response;
    }

    Winux::NetworkRequest request;
    request.Url = std::string(url);
    request.Timeout = std::chrono::milliseconds(30000);

    try {
        auto result = Winux::Get<Winux::Network>().Send(std::move(request));
        if (result.Failed()) {
            response.Error = result.Message();
            return response;
        }

        const auto& networkResponse = result.Value();
        response.StatusCode = networkResponse.StatusCode;
        if (!networkResponse.Body.empty()) {
            response.Body.assign(
                reinterpret_cast<const char*>(networkResponse.Body.data()),
                networkResponse.Body.size());
        }
    } catch (const std::exception& exception) {
        response.Error = exception.what();
    }

    return response;
}

}