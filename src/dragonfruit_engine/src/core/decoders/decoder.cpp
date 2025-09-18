#include "dragonfruit_engine/core/decoders/decoder.hpp"

#include "dragonfruit_engine/exception.hpp"

namespace dragonfruit {
void Decoder::Start() {
    if (m_decoder_thread.joinable()) {
        throw Exception(ErrorCode::INTERNAL_ERROR,
                        "Cannot start the decoder twice, an internal thread is already running");
    }

    m_decoder_thread = std::thread([&]() {
        while (DecodeFrame()) {
        }
    });
}

void Decoder::Stop() { m_decoder_thread.join(); }

Decoder::~Decoder() {
    if (m_decoder_thread.joinable()) {
        m_decoder_thread.join();
    }
}
}  // namespace dragonfruit