#pragma once

#include "dragonfruit_engine/core/decoders/decoder.hpp"
#include "dragonfruit_engine/core/parsers/wav_parser.hpp"

namespace dragonfruit {
class WavDecoder final : public Decoder {
   public:
    WavDecoder(Buffer& buffer, std::shared_ptr<DataSource> data_source);
    size_t NumFrames() override;
    void Seek(double seconds) override;

   private:
    bool DecodeFrame() override;
    WavParser m_parser;
    size_t m_cur_sample_data_offset;
};
}  // namespace dragonfruit
