#pragma once

#include "dragonfruit_engine/core/decoders/decoder.hpp"
#include "dragonfruit_engine/core/parsers/wav_parser.hpp"

namespace dragonfruit {
class WavDecoder final : public Decoder {
   public:
    WavDecoder(Buffer& buffer, DataSource& data_source);

   private:
    bool DecodeFrame() override;
    WavParser m_parser;
};
}  // namespace dragonfruit