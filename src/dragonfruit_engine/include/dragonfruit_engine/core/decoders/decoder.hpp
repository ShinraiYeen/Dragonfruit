#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include "dragonfruit_engine/core/engine_state.hpp"
#include "dragonfruit_engine/core/io/data_source.hpp"

namespace dragonfruit {
/**
 * @brief Abstract threaded decoder class.
 *
 */
class Decoder {
   public:
    virtual ~Decoder();

    void Start();
    void Seek(double seconds);
    void Stop();

    virtual size_t NumFrames() = 0;

   protected:
    Decoder(EngineState& state, std::unique_ptr<DataSource> data_source)
        : m_data_source(std::move(data_source)), m_state(state) {}

    virtual std::optional<std::vector<uint8_t>> DecodeFrame() = 0;
    virtual void SeekImpl(double seconds) = 0;

    std::unique_ptr<DataSource> m_data_source;  // Data source must be owned by the decoder

   private:
    void Pause(bool pause);

    std::thread m_decoder_thread;
    std::condition_variable m_thread_cond;
    std::mutex m_mutex;

    std::atomic<bool> m_shutdown = false;
    std::atomic<bool> m_paused = false;

    EngineState& m_state;
};
}  // namespace dragonfruit
