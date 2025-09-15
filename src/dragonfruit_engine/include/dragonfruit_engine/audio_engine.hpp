#pragma once

#include <pulse/pulseaudio.h>
#include <pulse/simple.h>

#include <memory>

#include "dragonfruit_engine/sound.hpp"

namespace dragonfruit {

struct EngineState {
    size_t offset = 0;        // Offset in bytes from the sample data to begin writing at
    bool is_finished = true;  // Whether the current stream has been finished or not.
    std::shared_ptr<Sound> sound;
};

/**
 * @brief Engine for playing sounds. Uses the PulseAudio API as a backend.
 *
 */
class AudioEngine {
   public:
    AudioEngine();
    ~AudioEngine();

    void PlayAsync(std::shared_ptr<Sound> sound);
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

    // Keeps track of the state of the currently playing song
    EngineState m_engine_state;
};
}  // namespace dragonfruit