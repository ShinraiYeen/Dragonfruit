#pragma once

#include <exception>
#include <string>

namespace dragonfruit {
enum class ErrorCode { INTERNAL_ERROR, INVALID_FORMAT, IO_ERROR };

class Exception : public std::exception {
   public:
    Exception() noexcept {}
    Exception(ErrorCode errorCode, const std::string& message) noexcept : m_message(message), m_error_code(errorCode) {}

    const char* what() const noexcept override;
    inline ErrorCode GetErrorCode() const noexcept { return m_error_code; }
    const std::string GetErrorCodeString() const noexcept;

   private:
    std::string m_message;
    ErrorCode m_error_code;
};
}  // namespace dragonfruit
