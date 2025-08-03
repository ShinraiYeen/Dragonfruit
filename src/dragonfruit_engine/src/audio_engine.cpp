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
            if (state == PA_STREAM_TERMINATED || state == PA_STREAM_FAILED) { break; }

            pa_threaded_mainloop_wait(mainloop);
        }

        pa_stream_unref(stream);
    }
}

AudioEngine::AudioEngine() {
    // Initialize threaded mainloop
    mainloop_ = pa_threaded_mainloop_new();
    if (!mainloop_) { throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to inqitialize pulse main loop"); }

    // Create context for threaded mainloop
    mainloop_api_ = pa_threaded_mainloop_get_api(mainloop_);
    context_ = pa_context_new(mainloop_api_, "dragonfruit-engine");
    if (!context_) { throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to create pulse context"); }

    pa_context_set_state_callback(context_, ContextStateCallback, mainloop_);

    // Start the main loop
    if (pa_threaded_mainloop_start(mainloop_) < 0) {
        throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to start pulse main loop thread");
    }

    pa_threaded_mainloop_lock(mainloop_);

    // Connect PulseAudio context to threaded mainloop
    if (pa_context_connect(context_, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        pa_threaded_mainloop_unlock(mainloop_);
        throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to connect pulse context");
    }

    // Wait until context is ready or failed in a blocking manner. The context state change call back should signal to
    // the mainloop once it has been called.
    while (true) {
        pa_context_state_t state = pa_context_get_state(context_);
        if (state == PA_CONTEXT_READY) { break; }

        if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED) {
            pa_threaded_mainloop_unlock(mainloop_);
            throw Exception(ErrorCode::INTERNAL_ERROR, "Failed to connect pulse context");
        }

        pa_threaded_mainloop_wait(mainloop_);
    }

    pa_threaded_mainloop_unlock(mainloop_);
}

AudioEngine::~AudioEngine() {
    pa_threaded_mainloop_lock(mainloop_);

    // Destroy stream
    AwaitStreamDisconnect(mainloop_, stream_);

    // Destroy the context (this is a blocking call, no awaiting required)
    pa_context_disconnect(context_);
    pa_context_unref(context_);
    pa_threaded_mainloop_unlock(mainloop_);

    // Destroy the threaded mainloop
    pa_threaded_mainloop_stop(mainloop_);
    pa_threaded_mainloop_free(mainloop_);
}

void AudioEngine::PlayAsync(std::shared_ptr<Sound> sound) {
    pa_threaded_mainloop_lock(mainloop_);

    // If there is already a stream setup, we will have to disconnect and create a new one

    AwaitStreamDisconnect(mainloop_, stream_);
    stream_ = nullptr;

    // Setup stream
    sample_spec_.channels = sound->Channels();
    sample_spec_.format = utils::GetPulseFormat(sound->Format(), sound->BitDepth());
    sample_spec_.rate = sound->SampleRate();

    if (sample_spec_.format == pa_sample_format::PA_SAMPLE_INVALID) {
        throw Exception(ErrorCode::INVALID_FORMAT, "Invalid WAV format");
    }

    // Setup internal engine state
    engine_state_.offset = 0;
    engine_state_.is_finished = false;
    engine_state_.sound = sound;

    // Create a new stream connect to the context and hook the state change and write callbacks for async functionality
    stream_ = pa_stream_new(context_, "Playback", &sample_spec_, nullptr);
    pa_stream_set_write_callback(stream_, StreamWriteCallback, &engine_state_);
    pa_stream_set_state_callback(stream_, StreamStateCallback, mainloop_);

    // Connect the stream to the pulse server in playback mode
    if (pa_stream_connect_playback(
            stream_, nullptr, nullptr,
            static_cast<pa_stream_flags_t>(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_AUTO_TIMING_UPDATE |
                                           PA_STREAM_ADJUST_LATENCY),
            nullptr, nullptr) < 0) {
        throw Exception(ErrorCode::INTERNAL_ERROR, "Unable to connect pulse stream");
    }

    // Wait for stream to be ready
    while (true) {
        pa_stream_state_t state = pa_stream_get_state(stream_);
        if (state == PA_STREAM_READY) { break; }

        if (state == PA_STREAM_FAILED || state == PA_STREAM_TERMINATED) {
            pa_threaded_mainloop_unlock(mainloop_);
            throw Exception(ErrorCode::INTERNAL_ERROR, "Stream failed to start");
        }

        pa_threaded_mainloop_wait(mainloop_);
    }

    sink_idx_ = pa_stream_get_index(stream_);

    pa_threaded_mainloop_unlock(mainloop_);
}

void AudioEngine::Pause(bool pause) {
    pa_threaded_mainloop_lock(mainloop_);
    pa_stream_cork(stream_, pause, nullptr, nullptr);
    pa_threaded_mainloop_unlock(mainloop_);
}

bool AudioEngine::IsFinished() { return engine_state_.is_finished; }

double AudioEngine::GetCurrentSongTime() {
    pa_threaded_mainloop_lock(mainloop_);

    pa_stream_update_timing_info(stream_, nullptr, nullptr);
    const pa_timing_info* timing_info = pa_stream_get_timing_info(stream_);

    // If we haven't received an update from the server yet, it's possible that the timing info is not yet valid. In
    // this case return 0.0, subsequent calls should work properly.
    if (!timing_info) {
        pa_threaded_mainloop_unlock(mainloop_);
        return 0.0;
    }

    int64_t played_offset = static_cast<int64_t>(engine_state_.offset) -
                            static_cast<int64_t>(timing_info->write_index - timing_info->read_index);

    played_offset = std::clamp(played_offset, int64_t(0), static_cast<int64_t>(engine_state_.sound->SampleDataSize()));

    pa_threaded_mainloop_unlock(mainloop_);

    return static_cast<double>(played_offset) / pa_bytes_per_second(&sample_spec_);
}

double AudioEngine::GetTotalSongTime() {
    pa_threaded_mainloop_lock(mainloop_);
    double total_song_time =
        static_cast<double>(pa_bytes_to_usec(engine_state_.sound->SampleDataSize(), &sample_spec_)) / 1000000.0;
    pa_threaded_mainloop_unlock(mainloop_);
    return total_song_time;
}

void AudioEngine::Seek(double seconds) {
    // Convert seconds into the amount of bytes to seek back
    size_t bytes_per_second = pa_bytes_per_second(&sample_spec_);

    pa_threaded_mainloop_lock(mainloop_);
    pa_stream_cork(stream_, true, nullptr, nullptr);
    const pa_timing_info* timing_info = pa_stream_get_timing_info(stream_);

    // We haven't received a timing update from the server yet, therefore we cannot accurately seek. In this case, we
    // return early. It is most likely this only occurs during edge cases, but we should check just in case.
    if (!timing_info) {
        pa_threaded_mainloop_unlock(mainloop_);
        return;
    }
    int64_t still_in_buffer = timing_info->write_index - timing_info->read_index;

    double bytes_to_seek = seconds * bytes_per_second;
    double current_offset = engine_state_.offset - still_in_buffer;

    // The new offset should be clamped between 0 (the start of the audio data) and the end of the audio data to ensure
    // we do not accidentally set the offset to unreadable/uninitialized memory regions.
    size_t new_offset = static_cast<size_t>(
        std::clamp(current_offset + bytes_to_seek, 0.0, static_cast<double>(engine_state_.sound->SampleDataSize())));

    // Ensure the new offset is aligned to the frame size
    new_offset = new_offset - (new_offset % pa_frame_size(&sample_spec_));
    engine_state_.offset = new_offset;

    // Flush the current buffer so that we start at our new offset
    pa_stream_flush(stream_, nullptr, nullptr);
    pa_stream_cork(stream_, false, nullptr, nullptr);
    pa_stream_update_timing_info(stream_, nullptr, nullptr);
    pa_threaded_mainloop_unlock(mainloop_);
}

void AudioEngine::SetVolume(double volume) {
    double volume_clamped = std::clamp(volume, 0.0, 1.0);

    pa_volume_t pa_volume = PA_VOLUME_NORM * volume_clamped;
    pa_cvolume cvol;

    pa_threaded_mainloop_lock(mainloop_);
    pa_cvolume_set(&cvol, sample_spec_.channels, pa_volume);
    pa_context_set_sink_input_volume(context_, sink_idx_, &cvol, nullptr, nullptr);
    pa_threaded_mainloop_unlock(mainloop_);
}

}  // namespace dragonfruit