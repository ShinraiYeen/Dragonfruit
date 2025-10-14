#pragma once

#include "dragonfruit_engine/core/metadata.hpp"

namespace dragonfruit {
class Parser {
   public:
    Parser() = default;
    virtual ~Parser() = default;

    virtual Metadata ExportMetadata() const = 0;
};
}  // namespace dragonfruit
