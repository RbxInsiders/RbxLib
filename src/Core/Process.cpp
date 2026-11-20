#include <RbxLib/Core/Process.h>

#include <Winux/Winux.h>

#include <string>

namespace RbxLib::Core {

Result<void> Process::Initialize() {
    const auto result = Winux::Platform::Initialize();
    if (result.Failed()) {
        return Result<void>::Failure(result.Message());
    }
    return Result<void>::Success();
}

void Process::Shutdown() noexcept {
    Winux::Platform::Shutdown();
}

Result<std::optional<std::uint32_t>> Process::Find(
    const std::wstring_view name) {
    const auto result = Winux::Get<Winux::Process>().FindProcess(
        std::wstring(name));
    if (result.Failed()) {
        return Result<std::optional<std::uint32_t>>::Failure(result.Message());
    }
    return Result<std::optional<std::uint32_t>>::Success(result.Value());
}

Result<std::filesystem::path> Process::FindLocation(const std::uint32_t processId) {
    const auto result = Winux::Get<Winux::Process>().FindLocation(processId);
    if (result.Failed()) {
        return Result<std::filesystem::path>::Failure(result.Message());
    }
    return Result<std::filesystem::path>::Success(result.Value());
}

Result<std::uintptr_t> Process::FindModule(
    const std::filesystem::path& name,
    const std::uint32_t processId) {
    const auto result = Winux::Get<Winux::Process>().FindModule(name, processId);
    if (result.Failed()) {
        return Result<std::uintptr_t>::Failure(result.Message());
    }
    return Result<std::uintptr_t>::Success(result.Value());
}

}