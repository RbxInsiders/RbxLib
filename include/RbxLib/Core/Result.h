#pragma once

#include <optional>
#include <string>
#include <utility>

namespace RbxLib::Core {

enum class ResultStatus {
    Success,
    Failure
};

template <typename TValue>
class Result {
public:
    [[nodiscard]] static Result Success(TValue value) {
        return Result(ResultStatus::Success, std::move(value), {});
    }

    [[nodiscard]] static Result Failure(std::string message) {
        return Result(ResultStatus::Failure, std::nullopt, std::move(message));
    }

    [[nodiscard]] ResultStatus Status() const noexcept {
        return m_status;
    }

    [[nodiscard]] bool Succeeded() const noexcept {
        return m_status == ResultStatus::Success;
    }

    [[nodiscard]] bool Failed() const noexcept {
        return m_status == ResultStatus::Failure;
    }

    [[nodiscard]] bool HasValue() const noexcept {
        return Succeeded();
    }

    explicit operator bool() const noexcept {
        return Succeeded();
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

    [[nodiscard]] const std::string& Message() const noexcept {
        return m_message;
    }

    [[nodiscard]] const std::string& Error() const noexcept {
        return m_message;
    }

private:
    Result(ResultStatus status, std::optional<TValue> value, std::string message)
        : m_status(status), m_value(std::move(value)), m_message(std::move(message)) {}

    ResultStatus m_status;
    std::optional<TValue> m_value;
    std::string m_message;
};

template <>
class Result<void> {
public:
    [[nodiscard]] static Result Success() {
        return Result(ResultStatus::Success, {});
    }

    [[nodiscard]] static Result Failure(std::string message) {
        return Result(ResultStatus::Failure, std::move(message));
    }

    [[nodiscard]] ResultStatus Status() const noexcept {
        return m_status;
    }

    [[nodiscard]] bool Succeeded() const noexcept {
        return m_status == ResultStatus::Success;
    }

    [[nodiscard]] bool Failed() const noexcept {
        return m_status == ResultStatus::Failure;
    }

    [[nodiscard]] bool HasValue() const noexcept {
        return Succeeded();
    }

    explicit operator bool() const noexcept {
        return Succeeded();
    }

    [[nodiscard]] const std::string& Message() const noexcept {
        return m_message;
    }

    [[nodiscard]] const std::string& Error() const noexcept {
        return m_message;
    }

private:
    Result(ResultStatus status, std::string message)
        : m_status(status), m_message(std::move(message)) {}

    ResultStatus m_status;
    std::string m_message;
};

}