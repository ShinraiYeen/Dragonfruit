#include "dragonfruit_engine/core/io/file_data_source.hpp"

#include <format>

#include "dragonfruit_engine/exception.hpp"

namespace dragonfruit {
FileDataSource::FileDataSource(const std::string& filepath) : m_filepath(filepath) {
    m_file.open(m_filepath, std::ios::binary);
    if (!m_file.is_open()) {
        throw Exception(ErrorCode::IO_ERROR, std::format("Unable to read file {}", m_filepath));
    }

    // Get file size immediately
    m_file.seekg(0, std::ios::end);
    m_file_size = m_file.tellg();
    m_file.seekg(0, std::ios::beg);
}

size_t FileDataSource::Read(uint8_t* buffer, size_t max_bytes) {
    m_file.read(reinterpret_cast<char*>(buffer), max_bytes);
    return static_cast<size_t>(m_file.gcount());
}

void FileDataSource::Seek(size_t position) { m_file.seekg(position, std::ios::beg); }

size_t FileDataSource::Tell() { return m_file.tellg(); }

size_t FileDataSource::Size() { return m_file_size; }

bool FileDataSource::EndOfFile() { return Tell() >= Size(); }
}  // namespace dragonfruit
