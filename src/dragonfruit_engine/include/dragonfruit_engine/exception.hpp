#pragma once

#include <exception>
#include <string>

namespace dragonfruit {
enum class ErrorCode { INTERNAL_ERROR, INVALID_FORMAT, IO_ERROR };

class Exception : public std::exception {
   public:
    Exception() noexcept {}
    Exception(ErrorCode errorCode, const std::string& message) noexcept : m_message(message), m_errorCode(errorCode) {}

    const char* what() const noexcept override { return m_message.c_str(); }

   private:
    std::string m_message;
    ErrorCode m_errorCode;
};
}  // namespace dragonfruit