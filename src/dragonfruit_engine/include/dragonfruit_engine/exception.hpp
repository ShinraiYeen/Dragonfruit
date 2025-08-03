#pragma once

#include <exception>
#include <format>
#include <string>

namespace dragonfruit {
enum class ErrorCode { INTERNAL_ERROR, INVALID_FORMAT, IO_ERROR };

class Exception : public std::exception {
   public:
    Exception() noexcept {}
    Exception(ErrorCode errorCode, const std::string& message) noexcept : message_(message), error_code_(errorCode) {}

    const char* what() const noexcept override;
    inline ErrorCode GetErrorCode() const noexcept { return error_code_; }
    const std::string GetErrorCodeString() const noexcept;

   private:
    std::string message_;
    ErrorCode error_code_;
};
}  // namespace dragonfruit