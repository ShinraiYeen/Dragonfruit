#pragma once

#include <memory>

#include "dragonfruit_engine/core/io/file_data_source.hpp"
#include "dragonfruit_engine/core/parsers/parser.hpp"

namespace dragonfruit {
class TagParser {
   public:
    TagParser(FileDataSource& file);
    Metadata ExportMetadata() const { return m_parser->ExportMetadata(); }

   private:
    std::unique_ptr<Parser> m_parser;
};
}  // namespace dragonfruit
