#include <RbxLib/Instances/Instance.h>

namespace RbxLib::Roblox {

Instance::Instance() = default;

Instance::Instance(std::shared_ptr<void> session)
    : m_session(std::move(session)) {}

bool Instance::IsValid() const noexcept {
    return m_session != nullptr;
}

std::shared_ptr<void> Instance::Session() const noexcept {
    return m_session;
}

}
