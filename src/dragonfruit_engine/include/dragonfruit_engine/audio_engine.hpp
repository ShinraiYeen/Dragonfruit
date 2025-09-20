#pragma once

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include <memory>

#include "dragonfruit_engine/core/buffer.hpp"
#include "dragonfruit_engine/core/decoders/decoder.hpp"
#include "dragonfruit_engine/core/io/data_source.hpp"

namespace dragonfruit {

/**
 * This is something that's important, trust me.
 */
struct EngineState {
    size_t offset = 0;        // Local offset of bytes into the sample data
    bool is_finished = true;  // Whether the current stream has been finished or not.
    Buffer& buffer;
};

/**
 * @brief Engine for playing sounds. Uses the PulseAudio API as a backend.
 *
 */
class AudioEngine {
   public:
    AudioEngine();
    ~AudioEngine();

    void PlayAsync(std::shared_ptr<DataSource> data_source);
    void Pause(bool pause);
    bool IsFinished();
    double GetTotalSongTime();
    double GetCurrentSongTime();
    void Seek(double seconds);
    void SetVolume(double volume);
    bool IsPaused();

   private:
    // PulseAudio state variables
    pa_threaded_mainloop* m_mainloop = nullptr;
    pa_mainloop_api* m_mainloop_api = nullptr;
    pa_context* m_context = nullptr;
    pa_stream* m_stream = nullptr;
    pa_sample_spec m_sample_spec;
    uint32_t m_sink_idx = 0;

    Buffer m_buffer;
    EngineState m_engine_state;
    std::unique_ptr<Decoder> m_decoder;
};
}  // namespace dragonfruit
