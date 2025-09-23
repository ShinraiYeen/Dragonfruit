#pragma once

#include <optional>

#include "dragonfruit_engine/core/decoders/decoder.hpp"
#include "dragonfruit_engine/core/parsers/wav_parser.hpp"

namespace dragonfruit {
class WavDecoder final : public Decoder {
   public:
    WavDecoder(Buffer& buffer, std::unique_ptr<DataSource> data_source);
    size_t NumFrames() override;
    void SeekImpl(double seconds) override;

   private:
    std::optional<std::vector<uint8_t>> DecodeFrame() override;
    WavParser m_parser;
    size_t m_cur_sample_data_offset;
};
}  // namespace dragonfruit
