#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

using namespace ftxui;

class PlayingIndicatorBase : public ComponentBase {
   public:
    PlayingIndicatorBase(const std::vector<std::string>& frames, int64_t frame_time = 150, int start_frame = 0);

    Element OnRender() override;

   private:
    std::vector<std::string> frames_;
    int frame_ = 0;
    int64_t frame_time_;
    std::chrono::steady_clock::time_point last_tick_;
};

inline Component PlayingIndicator(const std::vector<std::string>& frames, size_t frame_time = 150,
                                  int start_frame = 0) {
    return Make<PlayingIndicatorBase>(frames, frame_time, start_frame);
}