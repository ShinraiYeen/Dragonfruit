#include "dragonfruit_engine/audio_engine.hpp"

#include <pulse/def.h>
#include <pulse/error.h>
#include <pulse/sample.h>
#include <pulse/stream.h>
#include <pulse/thread-mainloop.h>
#include <pulse/volume.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "dragonfruit_engine/core/buffer.hpp"
#include "dragonfruit_engine/core/decoders/wav_decoder.hpp"
#include "dragonfruit_engine/exception.hpp"
#include "dragonfruit_engine/logging/logger.hpp"

namespace dragonfruit {

namespace {
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
    EngineState* state = static_cast<EngineState*>(userData);

    const size_t frame_size = pa_frame_size(&state->spec);
    const size_t num_frames = std::floor(length / frame_size);
    std::vector<uint8_t> chunk;
    chunk.reserve(length);

    for (size_t i = 0; i < num_frames; i++) {
        std::optional<BufferItem> item = state->buffer.Pop();
        if (!item.has_value()) {
            chunk.insert(chunk.end(), frame_size, 0);
            continue;
        }

        switch (item.value().item_type) {
            case ItemType::DecodedFrame:
                chunk.insert(chunk.end(), item.value().data.begin(), item.value().data.end());
                state->offset += item.value().data.size();
                break;
            case ItemType::DecodeFinished:
                chunk.insert(chunk.end(), frame_size, 0);
                state->is_finished = true;
                break;
        }
    }

    pa_stream_write(stream, chunk.data(), chunk.size(), nullptr, 0, PA_SEEK_RELATIVE);

    if (state->is_finished) pa_stream_cork(stream, 1, nullptr, nullptr);
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

}  // namespace

AudioEngine::AudioEngine(const size_t buffer_size) : m_buffer(buffer_size), m_engine_state(m_buffer, m_sample_spec) {
    Logger::Get()->info("Initializing audio engine");

    // Initialize threaded mainloop
    m_mainloop = pa_threaded_mainloop_new();
    if (!m_mainloop) {
        throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to initialize pulse main loop");
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

    // Create the main audio stream and hook the appropriate callbacks

    m_sample_spec.channels = 2;
    m_sample_spec.format = pa_sample_format::PA_SAMPLE_S16LE;
    m_sample_spec.rate = 44100;

    if (m_sample_spec.format == pa_sample_format::PA_SAMPLE_INVALID) {
        throw Exception(ErrorCode::INVALID_FORMAT, "Invalid WAV format");
    }

    // Create a new stream connect to the context and hook the state change and write callbacks for async functionality
    m_stream = pa_stream_new(m_context, "Playback", &m_sample_spec, nullptr);
    pa_stream_set_write_callback(m_stream, StreamWriteCallback, &m_engine_state);
    pa_stream_set_state_callback(m_stream, StreamStateCallback, m_mainloop);

    // Connect the stream to the pulse server in playback mode
    if (pa_stream_connect_playback(
            m_stream, nullptr, nullptr,
            static_cast<pa_stream_flags_t>(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE |
                                           PA_STREAM_ADJUST_LATENCY | PA_STREAM_START_CORKED),
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

    Logger::Get()->info("Finished initializing audio engine");
}

AudioEngine::~AudioEngine() {
    Logger::Get()->info("Quitting audio engine");
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

void AudioEngine::PlayAsync(std::unique_ptr<DataSource> data_source) {
    pa_threaded_mainloop_lock(m_mainloop);

    m_decoder.reset(new WavDecoder(m_buffer, std::move(data_source)));
    m_decoder->Start();
    m_engine_state.Reset();

    pa_stream_cork(m_stream, 0, nullptr, nullptr);

    pa_threaded_mainloop_unlock(m_mainloop);
}

void AudioEngine::Pause(bool pause) {
    pa_threaded_mainloop_lock(m_mainloop);
    pa_stream_cork(m_stream, pause, nullptr, nullptr);
    pa_threaded_mainloop_unlock(m_mainloop);
}

bool AudioEngine::IsFinished() {
    pa_threaded_mainloop_lock(m_mainloop);
    bool finished = m_engine_state.is_finished;
    pa_threaded_mainloop_unlock(m_mainloop);
    return finished;
}

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

    played_offset = std::clamp(played_offset, int64_t(0), static_cast<int64_t>(m_decoder->NumFrames() * 4));

    pa_threaded_mainloop_unlock(m_mainloop);

    return static_cast<double>(played_offset) / pa_bytes_per_second(&m_sample_spec);
}

double AudioEngine::GetTotalSongTime() {
    pa_threaded_mainloop_lock(m_mainloop);
    // TODO: Frame size is hard coded right now, must change later
    double total_song_time =
        static_cast<double>(pa_bytes_to_usec(m_decoder->NumFrames() * 4, &m_sample_spec)) / 1000000.0;
    pa_threaded_mainloop_unlock(m_mainloop);
    return total_song_time;
}

void AudioEngine::Seek(double seconds) {
    const size_t frame_size = pa_frame_size(&m_sample_spec);
    const double song_time_seconds = GetTotalSongTime();
    const double seconds_clamped = std::clamp(seconds, 0.0, song_time_seconds);

    pa_threaded_mainloop_lock(m_mainloop);
    pa_stream_cork(m_stream, true, nullptr, nullptr);

    const size_t frame_index = std::floor(seconds_clamped * m_sample_spec.rate);
    m_engine_state.offset = frame_index * frame_size;
    m_decoder->Seek(seconds_clamped);

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
}

bool AudioEngine::IsPaused() {
    pa_threaded_mainloop_lock(m_mainloop);
    int paused = pa_stream_is_corked(m_stream);
    pa_threaded_mainloop_unlock(m_mainloop);

    return paused < 1 ? false : true;
}

}  // namespace dragonfruit
