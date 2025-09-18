#pragma once

#include <string>
#include <unordered_map>

#include "dragonfruit_engine/core/io/data_source.hpp"

namespace dragonfruit {

class WavParser {
   public:
    enum class WavFormatCode { PCM, IEEE_FLOAT, EXTENSIBLE, UNKNOWN };
    enum class ChunkCode { FMT, LIST, DATA, UNKNOWN };

    /**
     * @brief Construct a new Sound using a filepath to a WAV file to load.
     *
     * @param[in] filepath Filepath pointing to a valid WAV file.
     */
    WavParser(DataSource& data_source);
    ~WavParser();

    /**
     * @brief Returns the number of channels.
     *
     * @return Number of channels.
     */
    inline uint16_t Channels() const { return m_channels; }

    /**
     * @brief Returns the bit depth of a sample.
     *
     * @return Bit depth of a sample.
     */
    inline uint16_t BitDepth() const { return m_bit_depth; }

    /**
     * @brief Returns the sample rate in Hz.
     *
     * @return The sample rate in Hz
     */
    inline uint32_t SampleRate() const { return m_sample_rate; }

    /**
     * @brief Returns the value of an INFO metadata tag if it exists. If it does not exist, returns an empty string.
     *
     * @param tag The INFO tag.
     * @return Value of the tag or an empty string if it does not exist.
     */
    std::string Metadata(std::string tag) const;

    /**
     * @brief Returns the name of the song.
     *
     * @return Name of the song.
     */
    inline std::string Name() const { return Metadata("INAM"); }

    /**
     * @brief Returns the album name.
     *
     * @return The album name.
     */
    inline std::string Album() const { return Metadata("IPRD"); }

    /**
     * @brief Returns the artist(s) name.
     *
     * @return The artist(s) name.
     */
    inline std::string Artist() const { return Metadata("IART"); }

    /**
     * @brief Returns the song comments.
     *
     * @return The song comments.
     */
    inline std::string Comments() const { return Metadata("ICMT"); }

    /**
     * @brief Returns the song year.
     *
     * @return The song year.
     */
    inline std::string Year() const { return Metadata("ICRD"); }

    /**
     * @brief Returns the genre.
     *
     * @return The genre.
     */
    inline std::string Genre() const { return Metadata("IGNR"); }

    /**
     * @brief Returns the track number.
     *
     * @return The track number.
     */
    inline std::string TrackNumber() const { return Metadata("ITRK"); }

    inline WavFormatCode Format() const { return m_format; }

    inline size_t SampleDataOffest() const { return m_sample_data_offset; }

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
};
}  // namespace dragonfruit