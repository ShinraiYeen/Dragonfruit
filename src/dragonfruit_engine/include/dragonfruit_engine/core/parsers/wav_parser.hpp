#pragma once

#include <string>
#include <unordered_map>

#include "dragonfruit_engine/core/io/data_source.hpp"
#include "dragonfruit_engine/core/metadata.hpp"
#include "dragonfruit_engine/core/parsers/parser.hpp"

namespace dragonfruit {

class WavParser : public Parser {
   public:
    enum class WavFormatCode { PCM, IEEE_FLOAT, EXTENSIBLE, UNKNOWN };
    enum class ChunkCode { FMT, LIST, DATA, UNKNOWN };

    WavParser(DataSource& data_source);
    ~WavParser();

    inline uint16_t Channels() const { return m_channels; }
    inline uint16_t BitDepth() const { return m_bit_depth; }
    inline uint32_t SampleRate() const { return m_sample_rate; }
    Metadata ExportMetadata() const override;

    std::string GetTagValue(std::string tag) const;

    inline WavFormatCode Format() const { return m_format; }
    inline size_t SampleDataOffset() const { return m_sample_data_offset; }
    inline size_t SampleDataSize() const { return m_sample_data_size; }
    inline size_t NumFrames() const { return m_sample_data_size / ((m_bit_depth / 8) * m_channels); }

   private:
    struct ChunkHeader {
        char id[4];
        uint32_t size;
    } __attribute__((packed));

    struct FmtChunk {
        uint16_t audio_format;
        uint16_t num_channels;
        uint32_t frequency;
        uint32_t bytes_per_sec;
        uint16_t bytes_per_bloc;
        uint16_t bits_per_sample;
    } __attribute__((packed));

    struct FmtExtendedChunk {
        uint16_t valid_bits_per_sample;
        uint32_t channel_mask;
        char sub_format[16];
    } __attribute__((packed));

    struct RiffChunk {
        ChunkHeader header;
        char wav_id[4];
    } __attribute__((packed));

    struct InfoChunk {
        char info_id[4];
    };

    bool ReadChunk(DataSource& file);
    void ParseChunk(ChunkHeader header, DataSource& file);
    void HandleFmtChunk(DataSource& file, size_t size);
    void HandleDataChunk(DataSource& file, size_t size);
    void HandleListChunk(DataSource& file, size_t size);
    void HandleUnknownChunk(DataSource& file, size_t size);

    std::unordered_map<std::string, std::string> m_info_tags;

    // WAV format information
    unsigned int m_sample_rate;
    uint16_t m_channels;
    uint16_t m_bit_depth;
    WavFormatCode m_format;
    size_t m_sample_data_offset;
    size_t m_sample_data_size;
};
}  // namespace dragonfruit
