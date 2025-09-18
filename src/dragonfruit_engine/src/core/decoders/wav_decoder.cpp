#include "dragonfruit_engine/core/decoders/wav_decoder.hpp"

namespace dragonfruit {
WavDecoder::WavDecoder(Buffer& buffer, DataSource& data_source) : Decoder(buffer, data_source) {}

bool WavDecoder::DecodeFrame() {
    printf("Decoding a frame!\n");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}
}  // namespace dragonfruit