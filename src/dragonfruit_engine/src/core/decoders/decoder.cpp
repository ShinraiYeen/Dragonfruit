#include "dragonfruit_engine/core/decoders/decoder.hpp"

#include <pthread.h>

#include <mutex>
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

    m_state.decoder_finished.store(false);

    // Make sure the buffer is fresh before we start pushing decoded frames into it
    m_state.buffer.Abort(false);
    m_state.buffer.Clear();

    m_decoder_thread = std::thread([this] {
        pthread_setname_np(pthread_self(), "df-decoder");
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_thread_cond.wait(lock, [this] { return !m_paused || m_shutdown; });

            if (m_shutdown) break;

            std::vector<uint8_t> data = DecodeStep();

            // We should not hold onto the lock while we're attempting to push frames into the shared buffer. That is
            // just asking for a deadlock.
            lock.unlock();

            if (data.size() == 0) {
                Logger::Get()->debug("[Decoder] Finished decoding");
                m_state.decoder_finished.store(true);

                lock.lock();
                // Critical section as we are modifying a shared variable. Pause the decoder state once we've finished
                // decoding the given file.
                m_paused = true;
                continue;
            }

            m_state.buffer.Push(data);
        }
    });
}

void Decoder::Seek(double seconds) {
    Logger::Get()->debug("[Decoder] Seek to {}", seconds);
    m_state.decoder_finished.store(false);

    // Ensure the decoder is paused during the seek
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_paused = true;
    }

    // Buffer must be aborted (to signal to any blocked producers to continue) and then cleared. At this point, the
    // decoder thread should be waiting on its signal.
    m_state.buffer.Abort(true);
    m_state.buffer.Clear();
    m_state.buffer.Abort(false);

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
    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        m_shutdown = true;
        m_paused = false;
    }

    m_state.buffer.Abort(true);

    m_thread_cond.notify_one();

    m_state.decoder_finished.store(true);

    if (m_decoder_thread.joinable()) {
        m_decoder_thread.join();
    }

    // We should set the buffer back to its normal state. Just some nice housekeeping!
    m_state.buffer.Abort(false);
}

Decoder::~Decoder() { Logger::Get()->debug("[Decoder] Stopped"); }
}  // namespace dragonfruit
