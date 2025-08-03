#pragma once

#include <stdint.h>

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace dragonfruit {

enum class WavFormatCode { PCM, IEEE_FLOAT, EXTENSIBLE, UNKNOWN };

enum class ChunkCode { FMT, LIST, DATA, UNKNOWN };

struct ChunkHeader {
    char id[4];
    uint32_t size;
} __attribute__((packed));

struct FmtChunk {
    ushort audio_format;
    ushort num_channels;
    uint32_t frequency;
    uint32_t bytes_per_sec;
    ushort bytes_per_bloc;
    ushort bits_per_sample;
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

/**
 * @brief Parses, stores and manages the lifetime of a WAV file.
 *
 */
class Sound {
   public:
    /**
     * @brief Construct a new Sound using a filepath to a WAV file to load.
     *
     * @param[in] filepath Filepath pointing to a valid WAV file.
     */
    Sound(const std::string& filepath);
    ~Sound();

    /**
     * @brief Returns the number of channels.
     *
     * @return Number of channels.
     */
    inline ushort Channels() const { return channels_; }

    /**
     * @brief Returns the bit depth of a sample.
     *
     * @return Bit depth of a sample.
     */
    inline ushort BitDepth() const { return bit_depth_; }

    /**
     * @brief Returns the sample rate in Hz.
     *
     * @return The sample rate in Hz
     */
    inline uint32_t SampleRate() const { return sample_rate_; }

    /**
     * @brief Returns a pointer to the sample data.
     *
     * @return Pointer to sample data.
     */
    inline const char* SampleData() const { return sample_data_.data(); }

    /**
     * @brief Returns the size in bytes of the sample data.
     *
     * @return Size in bytes of the sample data.
     */
    inline uint32_t SampleDataSize() const { return sample_data_.size(); }

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

    inline WavFormatCode Format() const { return format_; }

   private:
    bool ReadChunk(std::ifstream& file);
    void ParseChunk(ChunkHeader header, std::ifstream& file);
    void HandleFmtChunk(std::ifstream& file, size_t size);
    void HandleDataChunk(std::ifstream& file, size_t size);
    void HandleListChunk(std::ifstream& file, size_t size);
    void HandleUnknownChunk(std::ifstream& file, size_t size);

    std::unordered_map<std::string, std::string> info_tags_;

    // WAV format information
    unsigned int sample_rate_;
    unsigned short channels_;
    unsigned short bit_depth_;
    WavFormatCode format_;

    std::vector<char> sample_data_;
};
}  // namespace dragonfruit