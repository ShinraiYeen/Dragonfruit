#include "components/playing_indicator.hpp"

PlayingIndicatorBase::PlayingIndicatorBase(const std::vector<std::string>& frames, size_t frame_time, int start_frame)
    : frames_(frames), frame_time_(frame_time), frame_(start_frame) {
    last_tick_ = std::chrono::steady_clock::now();
}

Element PlayingIndicatorBase::OnRender() {
    // Calculate if enough time has passed to update animation frame
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick_);
    if (duration.count() > frame_time_) {  // update every 150ms
        frame_ = (frame_ + 1) % frames_.size();
        last_tick_ = now;
    }

    return text(frames_[frame_]) | bold | color(Color::Green);
}