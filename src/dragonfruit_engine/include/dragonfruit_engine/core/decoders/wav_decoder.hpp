#pragma once

#include <optional>

#include "dragonfruit_engine/core/decoders/decoder.hpp"
#include "dragonfruit_engine/core/engine_state.hpp"
#include "dragonfruit_engine/core/parsers/wav_parser.hpp"

namespace dragonfruit {
class WavDecoder final : public Decoder {
   public:
    WavDecoder(EngineState& state, std::unique_ptr<DataSource> data_source);
    ~WavDecoder();
    size_t NumFrames() override;
    void SeekImpl(double seconds) override;

   private:
    std::optional<std::vector<uint8_t>> DecodeFrame() override;
    WavParser m_parser;
    size_t m_cur_sample_data_offset;
};
}  // namespace dragonfruit
