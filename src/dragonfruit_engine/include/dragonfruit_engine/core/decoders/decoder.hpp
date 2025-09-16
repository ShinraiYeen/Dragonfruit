#pragma once

#include <thread>
#include <vector>

#include "dragonfruit_engine/core/buffer.hpp"
#include "dragonfruit_engine/core/io/data_source.hpp"

namespace dragonfruit {
/**
 * @brief Abstract threaded decoder class.
 *
 */
template <typename T>
class Decoder {
   public:
    Decoder(Buffer<T>& buffer, DataSource& data_source);
    ~Decoder();

    void Start();
    void Stop();

   protected:
    virtual bool DecodeFrame(const std::vector<uint8_t> frame_data) = 0;

   private:
    std::thread m_decoder_thread;
    Buffer<T>& m_buffer;
    DataSource& m_data_source;
};
}  // namespace dragonfruit