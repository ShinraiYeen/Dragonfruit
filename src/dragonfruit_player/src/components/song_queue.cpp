#include "components/song_queue.hpp"

#include "components/progress_animations.hpp"

SongQueueBase::SongQueueBase(Player& player) : player_(player) {
    playing_indicator_ = PlayingIndicator(ProgressAnimations::DOTS1, 80);
}

Element SongQueueBase::OnRender() {
    std::vector<Element> elements;

    std::vector<std::filesystem::path>& queue = player_.GetSongQueue();
    size_t cur_idx = player_.GetCurrentSongIdx();

    for (size_t i = 0; i < queue.size(); i++) {
        Element song_entry = text(std::format("{}. {}", i + 1, queue[i].filename().string()));

        // Add additional decorators if this is the currently playing song
        if (i == cur_idx) {
            song_entry = song_entry | bold | focus;
            elements.push_back(hbox({playing_indicator_->Render(), separatorEmpty(), song_entry}));
        } else {
            song_entry = song_entry | color(Color::LightSlateGrey);
            elements.push_back(hbox({song_entry, filler()}));
        }
    }

    return vbox(std::move(elements)) | yframe | vscroll_indicator;
}