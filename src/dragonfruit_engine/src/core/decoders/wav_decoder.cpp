#include "dragonfruit_engine/core/decoders/wav_decoder.hpp"

namespace dragonfruit {
WavDecoder::WavDecoder(Buffer& buffer, DataSource& data_source) : Decoder(buffer, data_source), m_parser(data_source) {
    // Read RIFF header metadata
}

bool WavDecoder::DecodeFrame() {
    printf("Decoding a frame!\n");
    m_buffer.Push({0, 1, 2, 3});
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return true;
}
}  // namespace dragonfruit