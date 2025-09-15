#include "dragonfruit_engine/audio_engine.hpp"

#include <pulse/error.h>
#include <pulse/volume.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <thread>

#include "dragonfruit_engine/exception.hpp"
#include "dragonfruit_engine/utils.hpp"

namespace dragonfruit {

// Callback for context state changes
void ContextStateCallback(pa_context* context, void* userdata) {
    (void)context;  // Suppress unused warning
    pa_threaded_mainloop_signal(reinterpret_cast<pa_threaded_mainloop*>(userdata), 0);
}

// Callback for stream state changes
void StreamStateCallback(pa_stream* stream, void* userdata) {
    (void)stream;  // Suppress unused warning
    pa_threaded_mainloop_signal(static_cast<pa_threaded_mainloop*>(userdata), 0);
}

// Stream write callback
void StreamWriteCallback(pa_stream* stream, size_t length, void* userData) {
    EngineState* audio = static_cast<EngineState*>(userData);
    size_t remaining = audio->sound->SampleDataSize() - audio->offset;
    size_t bytesToWrite = std::min(length, remaining);

    if (bytesToWrite > 0) {
        pa_stream_write(stream, audio->sound->SampleData() + audio->offset, bytesToWrite, nullptr, 0, PA_SEEK_RELATIVE);
        audio->offset += bytesToWrite;
    } else {
        audio->is_finished = true;
        pa_stream_cork(stream, true, nullptr, nullptr);
    }
}

// Blocking call to wait for a stream to disconnect since pa_stream_disconnect is an async call.
void AwaitStreamDisconnect(pa_threaded_mainloop* mainloop, pa_stream* stream) {
    if (stream) {
        pa_stream_disconnect(stream);

        while (true) {
            pa_stream_state_t state = pa_stream_get_state(stream);
            if (state == PA_STREAM_TERMINATED || state == PA_STREAM_FAILED) {
                break;
            }

            pa_threaded_mainloop_wait(mainloop);
        }

        pa_stream_unref(stream);
    }
}

AudioEngine::AudioEngine() {
    // Initialize threaded mainloop
    m_mainloop = pa_threaded_mainloop_new();
    if (!m_mainloop) {
        throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to inqitialize pulse main loop");
    }

    // Create context for threaded mainloop
    m_mainloop_api = pa_threaded_mainloop_get_api(m_mainloop);
    m_context = pa_context_new(m_mainloop_api, "Dragonfruit");
    if (!m_context) {
        throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to create pulse context");
    }

    pa_context_set_state_callback(m_context, ContextStateCallback, m_mainloop);

    // Start the main loop
    if (pa_threaded_mainloop_start(m_mainloop) < 0) {
        throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to start pulse main loop thread");
    }

    pa_threaded_mainloop_lock(m_mainloop);

    // Connect PulseAudio context to threaded mainloop
    if (pa_context_connect(m_context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        pa_threaded_mainloop_unlock(m_mainloop);
        throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to connect pulse context");
    }

    // Wait until context is ready or failed in a blocking manner. The context state change call back should signal to
    // the mainloop once it has been called.
    while (true) {
        pa_context_state_t state = pa_context_get_state(m_context);
        if (state == PA_CONTEXT_READY) {
            break;
        }

        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            pa_threaded_mainloop_unlock(m_mainloop);
            throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to connect pulse context");
        }

        pa_threaded_mainloop_wait(m_mainloop);
    }

    pa_threaded_mainloop_unlock(m_mainloop);
}

AudioEngine::~AudioEngine() {
    pa_threaded_mainloop_lock(m_mainloop);

    // Destroy stream
    AwaitStreamDisconnect(m_mainloop, m_stream);

    // Destroy the context (this is a blocking call, no awaiting required)
    pa_context_disconnect(m_context);
    pa_context_unref(m_context);
    pa_threaded_mainloop_unlock(m_mainloop);

    // Destroy the threaded mainloop
    pa_threaded_mainloop_stop(m_mainloop);
    pa_threaded_mainloop_free(m_mainloop);
}

void AudioEngine::PlayAsync(std::shared_ptr<Sound> sound) {
    pa_threaded_mainloop_lock(m_mainloop);

    // If there is already a stream setup, we will have to disconnect and create a new one

    AwaitStreamDisconnect(m_mainloop, m_stream);
    m_stream = nullptr;

    // Setup stream
    m_sample_spec.channels = sound->Channels();
    m_sample_spec.format = utils::GetPulseFormat(sound->Format(), sound->BitDepth());
    m_sample_spec.rate = sound->SampleRate();

    if (m_sample_spec.format == pa_sample_format::PA_SAMPLE_INVALID) {
        throw Exception(ErrorCode::INVALID_FORMAT, "Invalid WAV format");
    }

    // Setup internal engine state
    m_engine_state.offset = 0;
    m_engine_state.is_finished = false;
    m_engine_state.sound = sound;

    // Create a new stream connect to the context and hook the state change and write callbacks for async functionality
    m_stream = pa_stream_new(m_context, "Playback", &m_sample_spec, nullptr);
    pa_stream_set_write_callback(m_stream, StreamWriteCallback, &m_engine_state);
    pa_stream_set_state_callback(m_stream, StreamStateCallback, m_mainloop);

    // Connect the stream to the pulse server in playback mode
    if (pa_stream_connect_playback(
            m_stream, nullptr, nullptr,
            static_cast<pa_stream_flags_t>(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE |
                                           PA_STREAM_ADJUST_LATENCY),
            nullptr, nullptr) < 0) {
        throw Exception(ErrorCode::INTERNAL_ERROR, "Unable to connect pulse stream");
    }

    // Wait for stream to be ready
    while (true) {
        pa_stream_state_t state = pa_stream_get_state(m_stream);
        if (state == PA_STREAM_READY) {
            break;
        }

        if (state == PA_STREAM_FAILED || state == PA_STREAM_TERMINATED) {
            pa_threaded_mainloop_unlock(m_mainloop);
            throw Exception(ErrorCode::INTERNAL_ERROR, "Stream failed to start");
        }

        pa_threaded_mainloop_wait(m_mainloop);
    }

    m_sink_idx = pa_stream_get_index(m_stream);

    pa_threaded_mainloop_unlock(m_mainloop);
}

void AudioEngine::Pause(bool pause) {
    pa_threaded_mainloop_lock(m_mainloop);
    pa_stream_cork(m_stream, pause, nullptr, nullptr);
    pa_threaded_mainloop_unlock(m_mainloop);
}

bool AudioEngine::IsFinished() { return m_engine_state.is_finished; }

double AudioEngine::GetCurrentSongTime() {
    pa_threaded_mainloop_lock(m_mainloop);

    pa_stream_update_timing_info(m_stream, nullptr, nullptr);
    const pa_timing_info* timing_info = pa_stream_get_timing_info(m_stream);

    // If we haven't received an update from the server yet, it's possible that the timing info is not yet valid. In
    // this case return 0.0, subsequent calls should work properly.
    if (!timing_info) {
        pa_threaded_mainloop_unlock(m_mainloop);
        return 0.0;
    }

    int64_t played_offset = static_cast<int64_t>(m_engine_state.offset) -
                            static_cast<int64_t>(timing_info->write_index - timing_info->read_index);

    played_offset = std::clamp(played_offset, int64_t(0), static_cast<int64_t>(m_engine_state.sound->SampleDataSize()));

    pa_threaded_mainloop_unlock(m_mainloop);

    return static_cast<double>(played_offset) / pa_bytes_per_second(&m_sample_spec);
}

double AudioEngine::GetTotalSongTime() {
    pa_threaded_mainloop_lock(m_mainloop);
    double total_song_time =
        static_cast<double>(pa_bytes_to_usec(m_engine_state.sound->SampleDataSize(), &m_sample_spec)) / 1000000.0;
    pa_threaded_mainloop_unlock(m_mainloop);
    return total_song_time;
}

void AudioEngine::Seek(double seconds) {
    // Convert seconds into the amount of bytes to seek back
    size_t bytes_per_second = pa_bytes_per_second(&m_sample_spec);

    pa_threaded_mainloop_lock(m_mainloop);
    pa_stream_cork(m_stream, true, nullptr, nullptr);
    const pa_timing_info* timing_info = pa_stream_get_timing_info(m_stream);

    // We haven't received a timing update from the server yet, therefore we cannot accurately seek. In this case, we
    // return early. It is most likely this only occurs during edge cases, but we should check just in case.
    if (!timing_info) {
        pa_threaded_mainloop_unlock(m_mainloop);
        return;
    }
    int64_t still_in_buffer = timing_info->write_index - timing_info->read_index;

    double bytes_to_seek = seconds * bytes_per_second;
    double current_offset = m_engine_state.offset - still_in_buffer;

    // The new offset should be clamped between 0 (the start of the audio data) and the end of the audio data to ensure
    // we do not accidentally set the offset to unreadable/uninitialized memory regions.
    size_t new_offset = static_cast<size_t>(
        std::clamp(current_offset + bytes_to_seek, 0.0, static_cast<double>(m_engine_state.sound->SampleDataSize())));

    // Ensure the new offset is aligned to the frame size
    new_offset = new_offset - (new_offset % pa_frame_size(&m_sample_spec));
    m_engine_state.offset = new_offset;

    // Flush the current buffer so that we start at our new offset
    pa_stream_flush(m_stream, nullptr, nullptr);
    pa_stream_cork(m_stream, false, nullptr, nullptr);
    pa_stream_update_timing_info(m_stream, nullptr, nullptr);
    pa_threaded_mainloop_unlock(m_mainloop);
}

void AudioEngine::SetVolume(double volume) {
    double volume_clamped = std::clamp(volume, 0.0, 1.0);

    pa_volume_t pa_volume = PA_VOLUME_NORM * volume_clamped;
    pa_cvolume cvol;

    pa_threaded_mainloop_lock(m_mainloop);
    pa_cvolume_set(&cvol, m_sample_spec.channels, pa_volume);
    pa_context_set_sink_input_volume(m_context, m_sink_idx, &cvol, nullptr, nullptr);
    pa_threaded_mainloop_unlock(m_mainloop);

    m_volume = volume_clamped;
}

double AudioEngine::GetVolume() { return m_volume; }

bool AudioEngine::IsPaused() {
    pa_threaded_mainloop_lock(m_mainloop);
    int paused = pa_stream_is_corked(m_stream);
    pa_threaded_mainloop_unlock(m_mainloop);

    return paused < 1 ? false : true;
}

}  // namespace dragonfruit