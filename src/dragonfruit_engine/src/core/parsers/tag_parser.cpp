#include "dragonfruit_engine/core/parsers/tag_parser.hpp"

#include <filesystem>
#include <memory>

#include "dragonfruit_engine/core/io/file_data_source.hpp"
#include "dragonfruit_engine/core/parsers/wav_parser.hpp"

namespace dragonfruit {
TagParser::TagParser(FileDataSource& file) {
    std::filesystem::path path(file.Filepath());
    std::filesystem::path ext = path.extension();

    if (ext == ".wav") {
        m_parser = std::make_unique<WavParser>(file);
    }
}
}  // namespace dragonfruit
