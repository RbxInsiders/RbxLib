#pragma once

#include <RbxLib/Core/RobloxError.h>

#include <optional>
#include <utility>

namespace RbxLib::Core {

template <typename TValue>
class Result {
public:
    [[nodiscard]] static Result Success(TValue value) {
        return Result(std::move(value));
    }

    [[nodiscard]] static Result Failure(RobloxError error) {
        return Result(std::move(error));
    }

    [[nodiscard]] bool HasValue() const noexcept {
        return m_value.has_value();
    }

    explicit operator bool() const noexcept {
        return HasValue();
    }

    [[nodiscard]] TValue& Value() & {
        return m_value.value();
    }

    [[nodiscard]] const TValue& Value() const& {
        return m_value.value();
    }

    [[nodiscard]] TValue&& Value() && {
        return std::move(m_value.value());
    }

    [[nodiscard]] const RobloxError& Error() const& {
        return m_error.value();
    }

private:
    explicit Result(TValue value)
        : m_value(std::move(value)) {}

    explicit Result(RobloxError error)
        : m_error(std::move(error)) {}

    std::optional<TValue> m_value;
    std::optional<RobloxError> m_error;
};

template <>
class Result<void> {
public:
    [[nodiscard]] static Result Success() {
        return Result();
    }

    [[nodiscard]] static Result Failure(RobloxError error) {
        return Result(std::move(error));
    }

    [[nodiscard]] bool HasValue() const noexcept {
        return !m_error.has_value();
    }

    explicit operator bool() const noexcept {
        return HasValue();
    }

    [[nodiscard]] const RobloxError& Error() const& {
        return m_error.value();
    }

private:
    Result() = default;

    explicit Result(RobloxError error)
        : m_error(std::move(error)) {}

    std::optional<RobloxError> m_error;
};

}