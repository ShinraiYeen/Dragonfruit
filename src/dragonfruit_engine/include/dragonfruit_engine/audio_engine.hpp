#pragma once

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include <memory>

#include "dragonfruit_engine/core/decoders/decoder.hpp"
#include "dragonfruit_engine/core/io/data_source.hpp"

namespace dragonfruit {

#define DRAGONFRUIT_DEFAULT_BUFFER_SIZE 4096

/**
 * @brief Engine for playing sounds. Uses the PulseAudio API as a backend.
 *
 */
class AudioEngine {
   public:
    AudioEngine(const size_t buffer_size = DRAGONFRUIT_DEFAULT_BUFFER_SIZE);
    ~AudioEngine();

    void PlayAsync(std::unique_ptr<DataSource> data_source);
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

    EngineState m_engine_state;
    std::unique_ptr<Decoder> m_decoder;
};
}  // namespace dragonfruit
