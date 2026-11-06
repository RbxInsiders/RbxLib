#include <RbxLib/Core/Process.h>

#include <Winux/Winux.h>

namespace RbxLib::Core {

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