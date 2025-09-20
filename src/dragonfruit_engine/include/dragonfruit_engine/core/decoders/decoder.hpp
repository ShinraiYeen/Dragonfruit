#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "dragonfruit_engine/core/buffer.hpp"
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
    void Stop();
    void Pause();
    void Resume();

    virtual void Seek(double seconds) = 0;
    virtual size_t NumFrames() = 0;

   protected:
    Decoder(Buffer& buffer, std::shared_ptr<DataSource> data_source) : m_data_source(data_source), m_buffer(buffer) {}

    virtual bool DecodeFrame() = 0;

    std::shared_ptr<DataSource> m_data_source;
    Buffer& m_buffer;

   private:
    std::thread m_decoder_thread;
    std::atomic<bool> m_running;
    std::condition_variable m_thread_cond;
    std::mutex m_mutex;
    bool m_paused = false;
};
}  // namespace dragonfruit
