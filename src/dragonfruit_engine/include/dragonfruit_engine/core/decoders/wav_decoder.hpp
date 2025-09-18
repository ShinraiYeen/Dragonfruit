#pragma once

#include "dragonfruit_engine/core/decoders/decoder.hpp"

namespace dragonfruit {
class WavDecoder final : public Decoder {
   public:
    WavDecoder(Buffer& buffer, DataSource& data_source);

   private:
    bool DecodeFrame() override;
};
}  // namespace dragonfruit