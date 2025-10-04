#include "dragonfruit_engine/core/decoders/wav_decoder.hpp"

#include "dragonfruit_engine/logging/logger.hpp"

namespace dragonfruit {
WavDecoder::WavDecoder(EngineState& state, std::unique_ptr<DataSource> data_source)
    : Decoder(state, std::move(data_source)), m_parser(m_data_source) {
    Logger::Get()->info("[WAV Decoder] Initialized with {}Hz/{} bit/{}-channel", m_parser.SampleRate(),
                        m_parser.BitDepth(), m_parser.Channels());

    m_cur_sample_data_offset = m_parser.SampleDataOffset();
    m_data_source->Seek(m_cur_sample_data_offset);
}

std::optional<std::vector<uint8_t>> WavDecoder::DecodeFrame() {
    if (m_data_source->Tell() >= m_parser.SampleDataOffset() + m_parser.SampleDataSize()) {
        return std::nullopt;
    }

    const size_t frame_size = (m_parser.BitDepth() / 8) * m_parser.Channels();
    std::vector<uint8_t> frame(frame_size);
    m_data_source->Read(frame.data(), frame_size);

    return frame;
}

void WavDecoder::SeekImpl(double seconds) {
    const size_t frame_size = (m_parser.BitDepth() / 8) * m_parser.Channels();
    // Calculate frame index based on frame size and the sample rate
    const size_t frame_index = seconds * m_parser.SampleRate();
    const size_t byte_offset = frame_index * frame_size;
    m_data_source->Seek(m_parser.SampleDataOffset() + byte_offset);
}

size_t WavDecoder::NumFrames() { return m_parser.NumFrames(); }

WavDecoder::~WavDecoder() { Stop(); }
}  // namespace dragonfruit
