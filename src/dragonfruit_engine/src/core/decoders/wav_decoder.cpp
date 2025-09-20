#include "dragonfruit_engine/core/decoders/wav_decoder.hpp"

namespace dragonfruit {
WavDecoder::WavDecoder(Buffer& buffer, std::shared_ptr<DataSource> data_source)
    : Decoder(buffer, data_source), m_parser(data_source) {
    m_cur_sample_data_offset = m_parser.SampleDataOffset();
    data_source->Seek(m_cur_sample_data_offset);
}

bool WavDecoder::DecodeFrame() {
    size_t frame_size = (m_parser.BitDepth() / 8) * m_parser.Channels();
    std::vector<uint8_t> frame(frame_size);
    m_data_source->Read(frame.data(), frame_size);
    m_buffer.Push(frame);
    return !(m_data_source->Tell() >= m_parser.SampleDataOffset() + m_parser.SampleDataSize());
}

void WavDecoder::Seek(double seconds) {
    const size_t frame_size = (m_parser.BitDepth() / 8) * m_parser.Channels();
    // Calculate frame index based on frame size and the sample rate
    const size_t frame_index = seconds * m_parser.SampleRate();
    const size_t byte_offset = frame_index * frame_size;
    m_data_source->Seek(m_parser.SampleDataOffset() + byte_offset);
}

size_t WavDecoder::NumFrames() { return m_parser.NumFrames(); }
}  // namespace dragonfruit
