#include "dragonfruit_engine/core/decoders/wav_decoder.hpp"

namespace dragonfruit {
WavDecoder::WavDecoder(Buffer& buffer, DataSource& data_source) : Decoder(buffer, data_source), m_parser(data_source) {
    m_cur_sample_data_offset = m_parser.SampleDataOffset();
    data_source.Seek(m_cur_sample_data_offset);
}

bool WavDecoder::DecodeFrame() {
    size_t frame_size = (m_parser.BitDepth() / 8) * m_parser.Channels();
    std::vector<uint8_t> frame(frame_size);
    m_data_source.Read(frame.data(), frame_size);
    m_buffer.Push(frame);
    return !m_data_source.Tell() >= m_parser.SampleDataOffset() + m_parser.SampleDataSize();
}
}  // namespace dragonfruit