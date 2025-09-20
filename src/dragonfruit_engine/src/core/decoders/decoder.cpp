#include "dragonfruit_engine/core/decoders/decoder.hpp"

#include <mutex>

#include "dragonfruit_engine/exception.hpp"

namespace dragonfruit {
void Decoder::Start() {
    if (m_decoder_thread.joinable()) {
        throw Exception(ErrorCode::INTERNAL_ERROR,
                        "Cannot start the decoder twice, an internal thread is already running");
    }

    m_running.store(true);
    m_decoder_thread = std::thread([&] {
        while (m_running.load()) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_thread_cond.wait(lock, [&] { return !m_paused; });
            if (!DecodeFrame()) break;
        }
    });
}

void Decoder::Pause() {
    std::scoped_lock<std::mutex> lock(m_mutex);
    m_paused = true;
}

void Decoder::Resume() {
    std::scoped_lock<std::mutex> lock(m_mutex);
    m_paused = false;
    // Wake up the decoder thread
    m_thread_cond.notify_one();
}

void Decoder::Stop() {
    m_running.store(false);
    if (m_decoder_thread.joinable()) {
        m_decoder_thread.join();
    }
}

Decoder::~Decoder() { Stop(); }
}  // namespace dragonfruit
