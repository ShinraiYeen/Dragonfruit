#pragma once

#include <pulse/sample.h>

#include <atomic>

#include "dragonfruit_engine/core/buffer.hpp"

namespace dragonfruit {
struct EngineState {
    EngineState(size_t buffer_capacity, const pa_sample_spec& spec) : buffer(buffer_capacity), spec(spec) {}

    Buffer buffer;
    std::atomic<bool> decoder_finished;
    std::atomic<bool> playing_finished;
    size_t read_offset = 0;
    const pa_sample_spec& spec;
};
}  // namespace dragonfruit
