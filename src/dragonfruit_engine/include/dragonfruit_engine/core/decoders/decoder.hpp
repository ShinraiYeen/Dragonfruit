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
class Decoder {
   public:
    virtual ~Decoder();

    void Start();
    void Stop();

   protected:
    Decoder(Buffer& buffer, DataSource& data_source) : m_buffer(buffer), m_data_source(data_source) {}

    virtual bool DecodeFrame() = 0;

   private:
    std::thread m_decoder_thread;
    Buffer& m_buffer;
    DataSource& m_data_source;
};
}  // namespace dragonfruit