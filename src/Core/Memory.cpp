#include <RbxLib/Core/Memory.h>

#include <Winux/Winux.h>

namespace RbxLib::Core {

Result<std::size_t> Memory::Read(
    const std::uint32_t processId,
    const std::uintptr_t remoteAddress,
    const std::span<std::byte> output) {
    const auto result = Winux::Get<Winux::Memory>().ReadMemory(
        processId, remoteAddress, output);
    if (result.Failed()) {
        return Result<std::size_t>::Failure(result.Message());
    }
    return Result<std::size_t>::Success(result.Value());
}

}