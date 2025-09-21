#include "dragonfruit_engine/core/decoders/decoder.hpp"

#include <pthread.h>

#include <mutex>

#include "dragonfruit_engine/exception.hpp"

namespace dragonfruit {
void Decoder::Start() {
    if (m_decoder_thread.joinable()) {
        throw Exception(ErrorCode::INTERNAL_ERROR,
                        "Cannot start the decoder twice, an internal thread is already running");
    }

    m_decoder_thread = std::thread([&] {
        pthread_setname_np(pthread_self(), "df-decoder");
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_thread_cond.wait(lock, [&] { return !m_paused || m_shutdown; });

            if (m_shutdown) break;
            lock.unlock();

            DecodeFrame();
        }
    });
}

void Decoder::Pause(bool pause) {
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_paused = pause;
    }
    m_thread_cond.notify_one();
}

void Decoder::Seek(double seconds) {
    Pause(true);
    SeekImpl(seconds);
    Pause(false);
}

void Decoder::Stop() {
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_buffer.Shutdown();
        m_shutdown = true;
    }
    m_thread_cond.notify_one();

    if (m_decoder_thread.joinable()) {
        m_decoder_thread.join();
    }
}

Decoder::~Decoder() { Stop(); }
}  // namespace dragonfruit
