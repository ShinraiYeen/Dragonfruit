#include "dragonfruit_engine/exception.hpp"

#include <format>

namespace dragonfruit {

static const std::string ErrorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::INTERNAL_ERROR:
            return "Internal Error";
        case ErrorCode::INVALID_FORMAT:
            return "Invalid Format";
        case ErrorCode::IO_ERROR:
            return "I/O Error";
    }
}

const char* Exception::what() const noexcept { return message_.c_str(); }

const std::string Exception::GetErrorCodeString() const noexcept { return ErrorCodeToString(error_code_); }
}  // namespace dragonfruit