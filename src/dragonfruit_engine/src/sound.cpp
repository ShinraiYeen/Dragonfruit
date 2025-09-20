#include "dragonfruit_engine/sound.hpp"

#include "dragonfruit_engine/exception.hpp"

namespace dragonfruit {

Sound::Sound(const std::string& filepath) {
    FileDataSource file(filepath);

    // Load RIFF metadata
    RiffChunk chunk;
    file.Read(reinterpret_cast<uint8_t*>(&chunk), sizeof(chunk));
    if (std::string(chunk.wav_id, 4) != "WAVE") {
        throw Exception(ErrorCode::INVALID_FORMAT, "File does not start with RIFF chunk");
    }

    while (ReadChunk(file));
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

void Sound::HandleFmtChunk(FileDataSource& file, size_t size) {
    if (size != 16 && size != 18 && size != 40) {
        throw Exception(ErrorCode::INVALID_FORMAT, "Malformed fmt chunk in WAV file");
    }

    FmtChunk chunk;
    file.Read(reinterpret_cast<uint8_t*>(&chunk), sizeof(chunk));
    m_channels = chunk.num_channels;
    m_sample_rate = chunk.frequency;
    m_bit_depth = chunk.bits_per_sample;

    // Determine audio format
    m_format = GetWavFormatCode(chunk.audio_format);

    // Check for extension if size is above 16 bytes
    if (size <= 16) return;
    uint16_t extensionSize;
    file.Read(reinterpret_cast<uint8_t*>(&extensionSize), sizeof(extensionSize));

    if (extensionSize == 0) return;

    // Otherwise, we must read the extended fmt data
    FmtExtendedChunk extendedChunk;
    file.Read(reinterpret_cast<uint8_t*>(&extendedChunk), sizeof(extendedChunk));

    m_format = GetWavFormatCode(extendedChunk.sub_format[1] << 8 | extendedChunk.sub_format[0]);
}

void Sound::HandleDataChunk(FileDataSource& file, size_t size) {
    m_sample_data.resize(size);
    file.Read(reinterpret_cast<uint8_t*>(m_sample_data.data()), size);
}

void Sound::HandleListChunk(FileDataSource& file, size_t size) {
    InfoChunk chunk;
    file.Read(reinterpret_cast<uint8_t*>(&chunk), 4);
    uint32_t bytesRead = 4;

    while (bytesRead < size) {
        ChunkHeader tag;
        file.Read(reinterpret_cast<uint8_t*>(&tag), sizeof(ChunkHeader));

        std::string value(tag.size, '\0');
        file.Read(reinterpret_cast<uint8_t*>(&value[0]), tag.size);
        m_info_tags[std::string(tag.id, 4)] = value;

        // Seek past padding if tag.size is odd
        if (tag.size % 2 != 0) {
            file.Seek(file.Tell() + 1);
            bytesRead++;
        }

        bytesRead += tag.size + sizeof(ChunkHeader);
    }
}

void Sound::HandleUnknownChunk(FileDataSource& file, size_t size) { file.Seek(file.Tell() + size); }

void Sound::ParseChunk(ChunkHeader header, FileDataSource& file) {
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
        file.Seek(file.Tell() + 1);
    }
}

bool Sound::ReadChunk(FileDataSource& file) {
    // Immediately return false if we're at the end of the file
    if (file.EndOfFile()) {
        return false;
    }

    // Read chunk header first
    ChunkHeader header;
    file.Read(reinterpret_cast<uint8_t*>(&header), sizeof(header));

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
