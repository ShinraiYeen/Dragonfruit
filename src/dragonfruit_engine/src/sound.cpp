#include "dragonfruit_engine/sound.hpp"

#include <fstream>
#include <iostream>
#include <optional>

#include "dragonfruit_engine/exception.hpp"

namespace dragonfruit {

Sound::Sound(const std::string& filepath) {
    std::ifstream file(filepath.c_str());

    // Load RIFF metadata
    RiffChunk chunk;
    file.read(reinterpret_cast<char*>(&chunk), sizeof(chunk));
    if (std::string(chunk.wav_id, 4) != "WAVE") {
        throw Exception(ErrorCode::INVALID_FORMAT, "File does not start with RIFF chunk");
    }

    while (ReadChunk(file));

    file.close();
}

Sound::~Sound() {}

ChunkCode GetChunkCode(const std::string& chunkID) {
    static const std::unordered_map<std::string, ChunkCode> idToChunkCode = {
        {"fmt ", ChunkCode::FMT}, {"LIST", ChunkCode::LIST}, {"data", ChunkCode::DATA}};

    if (!idToChunkCode.contains(chunkID)) return ChunkCode::UNKNOWN;

    return idToChunkCode.at(chunkID);
}

WavFormatCode GetWavFormatCode(uint16_t code) {
    switch (code) {
        case 0x0001:
            return WavFormatCode::PCM;
        case 0x0003:
            return WavFormatCode::IEEE_FLOAT;
        case 0xFFFE:
            return WavFormatCode::EXTENSIBLE;
        default:
            return WavFormatCode::UNKNOWN;
    }
}

void Sound::HandleFmtChunk(std::ifstream& file, size_t size) {
    if (size != 16 && size != 18 && size != 40) {
        throw Exception(ErrorCode::INVALID_FORMAT, "Malformed fmt chunk in WAV file");
    }

    FmtChunk chunk;
    file.read(reinterpret_cast<char*>(&chunk), sizeof(chunk));
    m_channels = chunk.num_channels;
    m_sample_rate = chunk.frequency;
    m_bit_depth = chunk.bits_per_sample;

    // Determine audio format
    m_format = GetWavFormatCode(chunk.audio_format);

    // Check for extension if size is above 16 bytes
    if (size <= 16) return;
    uint16_t extensionSize;
    file.read(reinterpret_cast<char*>(&extensionSize), sizeof(extensionSize));

    if (extensionSize == 0) return;

    // Otherwise, we must read the extended fmt data
    FmtExtendedChunk extendedChunk;
    file.read(reinterpret_cast<char*>(&extendedChunk), sizeof(extendedChunk));

    m_format = GetWavFormatCode(extendedChunk.sub_format[1] << 8 | extendedChunk.sub_format[0]);
}

void Sound::HandleDataChunk(std::ifstream& file, size_t size) {
    m_sample_data.resize(size);
    file.read(reinterpret_cast<char*>(m_sample_data.data()), size);
}

void Sound::HandleListChunk(std::ifstream& file, size_t size) {
    InfoChunk chunk;
    file.read(reinterpret_cast<char*>(&chunk), 4);
    uint32_t bytesRead = 4;

    while (bytesRead < size) {
        ChunkHeader tag;
        file.read(reinterpret_cast<char*>(&tag), sizeof(ChunkHeader));

        std::string value(tag.size, '\0');
        file.read(&value[0], tag.size);
        m_info_tags[std::string(tag.id, 4)] = value;

        // Seek past padding if tag.size is odd
        if (tag.size % 2 != 0) {
            file.seekg(1, std::ios::cur);
            bytesRead++;
        }

        bytesRead += tag.size + sizeof(ChunkHeader);
    }
}

void Sound::HandleUnknownChunk(std::ifstream& file, size_t size) { file.seekg(size, std::ios::cur); }

void Sound::ParseChunk(ChunkHeader header, std::ifstream& file) {
    ChunkCode code = GetChunkCode(std::string(header.id, 4));
    switch (code) {
        case ChunkCode::DATA: {
            HandleDataChunk(file, header.size);
            break;
        }

        case ChunkCode::FMT: {
            HandleFmtChunk(file, header.size);
            break;
        }

        case ChunkCode::LIST: {
            HandleListChunk(file, header.size);
            break;
        }

        case ChunkCode::UNKNOWN: {
            HandleUnknownChunk(file, header.size);
            break;
        }
    }

    // If chunk size was odd, we need to seek past the 1-byte padding
    if (header.size % 2 != 0) {
        file.seekg(1, std::ios::cur);
    }
}

bool Sound::ReadChunk(std::ifstream& file) {
    // Immediately return false if we're at the end of the file
    if (file.peek() == EOF) {
        return false;
    }

    // Read chunk header first
    ChunkHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    // Parse chunk
    ParseChunk(header, file);
    return true;
}

std::string Sound::Metadata(std::string tag) const {
    if (!m_info_tags.contains(tag)) {
        return "";
    }

    return m_info_tags.at(tag);
}
}  // namespace dragonfruit