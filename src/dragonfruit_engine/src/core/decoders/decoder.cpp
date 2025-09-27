#include "dragonfruit_engine/core/decoders/decoder.hpp"

#include <pthread.h>

#include <mutex>
#include <optional>
#include <thread>

#include "dragonfruit_engine/core/buffer.hpp"
#include "dragonfruit_engine/exception.hpp"
#include "dragonfruit_engine/logging/logger.hpp"

namespace dragonfruit {
void Decoder::Start() {
    Logger::Get()->debug("[Decoder] Start");

    if (m_decoder_thread.joinable()) {
        throw Exception(ErrorCode::INTERNAL_ERROR,
                        "Cannot start the decoder twice, an internal thread is already running");
    }

    // Make sure the buffer is fresh before we start pushing decoded frames into it
    m_buffer.Abort(false);
    m_buffer.Clear();

    m_decoder_thread = std::thread([this] {
        pthread_setname_np(pthread_self(), "df-decoder");
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_thread_cond.wait(lock, [this] { return !m_paused || m_shutdown; });

            if (m_shutdown) break;

            std::optional<std::vector<uint8_t>> frame = DecodeFrame();

            lock.unlock();
            if (!frame.has_value()) {
                Logger::Get()->debug("[Decoder] Finished decoding");
                m_buffer.Push(BufferItem(ItemType::DecodeFinished));
                lock.lock();
                m_paused = true;
                lock.unlock();
                continue;
            }

            m_buffer.Push(BufferItem(ItemType::DecodedFrame, frame.value()));
        }
    });
}

void Decoder::Seek(double seconds) {
    Logger::Get()->debug("[Decoder] Seek to {}", seconds);

    // Ensure the decoder is paused during the seek
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_paused = true;
    }

    // Buffer must be aborted (to signal to any blocked producers to continue) and then cleared. At this point, the
    // decoder thread should be waiting on its signal.
    m_buffer.Abort(true);
    m_buffer.Clear();
    m_buffer.Abort(false);

    // Acquire a lock to ensure the decoder thread isn't doing anything, then run seek implementation.
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        SeekImpl(seconds);
        m_paused = false;
    }

    // Once the buffer is empty and the seek call has been completed, we can notify the decoder thread to continue its
    // work.
    m_thread_cond.notify_one();
}

void Decoder::Stop() {
    m_buffer.Abort(true);
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_shutdown = true;
        m_paused = false;
    }

    m_thread_cond.notify_one();

    if (m_decoder_thread.joinable()) {
        m_decoder_thread.join();
    }

    // We should set the buffer back to its normal state. Just some nice housekeeping!
    m_buffer.Abort(false);
}

Decoder::~Decoder() { Logger::Get()->debug("[Decoder] Stopped"); }
}  // namespace dragonfruit
