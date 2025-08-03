#include "components/mini_player.hpp"

#include "components/playing_indicator.hpp"
#include "components/progress_animations.hpp"

std::string FormatSeconds(double seconds) {
    int total_seconds = static_cast<int>(seconds);
    int minutes = total_seconds / 60;
    int secs = total_seconds % 60;
    return std::format("{}:{:02}", minutes, secs);
}

MiniPlayerBase::MiniPlayerBase(Player& player) : player_(player) {
    play_indicator_1_ = PlayingIndicator(ProgressAnimations::GROW_VERTICAL, 100, 3);
    play_indicator_2_ = PlayingIndicator(ProgressAnimations::GROW_VERTICAL, 100, 0);
    play_indicator_3_ = PlayingIndicator(ProgressAnimations::GROW_VERTICAL, 100, 5);
}

Element MiniPlayerBase::OnRender() {
    double song_time = player_.GetCurrentSongTime();
    double total_song_time = player_.GetTotalSongTime();
    bool paused = player_.IsPaused();
    std::shared_ptr<dragonfruit::Sound> song = player_.GetCurrentSong();
    size_t total_songs = player_.GetSongQueue().size();
    size_t song_idx = player_.GetCurrentSongIdx();

    std::string song_name = song->Name().empty() ? player_.GetSongQueue()[song_idx].filename().string() : song->Name();

    Decorator progress_bar_decorator = color(LinearGradient(Color::CornflowerBlue, Color::BlueViolet));

    Element play_indicator =
        paused ? text("▁▁▁") | color(Color::Green)
               : hbox({play_indicator_1_->Render(), play_indicator_2_->Render(), play_indicator_3_->Render()});

    return vbox({
        hbox({
            text(FormatSeconds(song_time) + "/" + FormatSeconds(total_song_time)),
            separatorEmpty(),
            text(std::format("{} [{}/{}]", song_name, song_idx + 1, total_songs)) | flex_shrink,
            filler(),
            play_indicator,
            separatorEmpty(),
        }),
        hbox({
            text("["),
            gauge(song_time / total_song_time) | progress_bar_decorator | bgcolor(Color::GrayDark),
            text("]"),
        }),
    });
}