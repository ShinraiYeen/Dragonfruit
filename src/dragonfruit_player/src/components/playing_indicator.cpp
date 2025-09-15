#include "components/playing_indicator.hpp"

PlayingIndicatorBase::PlayingIndicatorBase(const std::vector<std::string>& frames, int64_t frame_time, int start_frame)
    : m_frames(frames), m_frame(start_frame), m_frame_time(frame_time) {
    m_last_tick = std::chrono::steady_clock::now();
}

Element PlayingIndicatorBase::OnRender() {
    // Calculate if enough time has passed to update animation frame
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_tick);
    if (duration.count() > m_frame_time) {  // update every 150ms
        m_frame = (m_frame + 1) % m_frames.size();
        m_last_tick = now;
    }

    return text(m_frames[m_frame]) | bold | color(Color::Green);
}