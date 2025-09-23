#include "dragonfruit_engine/core/decoders/decoder.hpp"

#include <pthread.h>

#include <mutex>
#include <optional>

#include "dragonfruit_engine/core/buffer.hpp"
#include "dragonfruit_engine/exception.hpp"

namespace dragonfruit {
void Decoder::Start() {
    if (m_decoder_thread.joinable()) {
        throw Exception(ErrorCode::INTERNAL_ERROR,
                        "Cannot start the decoder twice, an internal thread is already running");
    }

    m_buffer.SignalShutdown(false);
    m_buffer.Clear();

    m_decoder_thread = std::thread([&] {
        pthread_setname_np(pthread_self(), "df-decoder");
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_thread_cond.wait(lock, [&] { return !m_paused || m_shutdown; });

            if (m_shutdown) break;

            std::optional<std::vector<uint8_t>> frame = DecodeFrame();
            lock.unlock();

            if (!frame.has_value()) {
                m_buffer.Push(BufferItem(ItemType::DecodeFinished));
                break;
            }

            m_buffer.Push(BufferItem(ItemType::DecodedFrame, frame.value()));
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
    m_buffer.Clear();
    SeekImpl(seconds);
    Pause(false);
}

void Decoder::Stop() {
    m_buffer.SignalShutdown(true);
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_shutdown = true;
    }
    m_thread_cond.notify_one();

    if (m_decoder_thread.joinable()) {
        m_decoder_thread.join();
    }
}

Decoder::~Decoder() { Stop(); }
}  // namespace dragonfruit
