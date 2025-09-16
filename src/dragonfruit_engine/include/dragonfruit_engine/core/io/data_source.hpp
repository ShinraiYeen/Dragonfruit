#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace dragonfruit {
class DataSource {
   public:
    virtual size_t Read(uint8_t* buffer, size_t max_bytes) = 0;
    virtual void Seek(size_t position) = 0;
    virtual size_t Tell() = 0;
    virtual size_t Size() = 0;
};
}  // namespace dragonfruit