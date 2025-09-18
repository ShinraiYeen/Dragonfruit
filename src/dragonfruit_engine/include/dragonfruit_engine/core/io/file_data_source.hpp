#pragma once

#include <fstream>
#include <string>

#include "dragonfruit_engine/core/io/data_source.hpp"

namespace dragonfruit {
class FileDataSource : public DataSource {
   public:
    FileDataSource(const std::string& filepath);

    size_t Read(uint8_t* buffer, size_t max_bytes) override;
    void Seek(size_t position) override;
    size_t Tell() override;
    size_t Size() override;
    bool EndOfFile() override;

   private:
    std::ifstream m_file;
    size_t m_file_size;
    std::string m_filepath;
};
}  // namespace dragonfruit