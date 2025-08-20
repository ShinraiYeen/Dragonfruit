#include "components/mini_player.hpp"

#include "components/playing_indicator.hpp"
#include "components/progress_animations.hpp"

std::string FormatSeconds(double seconds) {
    int total_seconds = static_cast<int>(seconds);
    int minutes = total_seconds / 60;
    int secs = total_seconds % 60;
    return std::format("{}:{:02}", minutes, secs);
}

MiniPlayerBase::MiniPlayerBase(Player& player) : m_player(player) {
    m_play_indicator_1 = PlayingIndicator(ProgressAnimations::GROW_VERTICAL, 100, 3);
    m_play_indicator_2 = PlayingIndicator(ProgressAnimations::GROW_VERTICAL, 100, 0);
    m_play_indicator_3 = PlayingIndicator(ProgressAnimations::GROW_VERTICAL, 100, 5);
}

Element MiniPlayerBase::OnRender() {
    double song_time = m_player.GetCurrentSongTime();
    double total_song_time = m_player.GetTotalSongTime();
    bool paused = m_player.IsPaused();
    std::shared_ptr<dragonfruit::Sound> song = m_player.GetCurrentSong();
    size_t total_songs = m_player.GetSongQueue().size();
    size_t song_idx = m_player.GetCurrentSongIdx();

    std::string song_name = song->Name().empty() ? m_player.GetSongQueue()[song_idx].filename().string() : song->Name();

    Decorator progress_bar_decorator = color(LinearGradient(Color::CornflowerBlue, Color::BlueViolet));

    Element play_indicator =
        paused ? text("▁▁▁") | color(Color::Green)
               : hbox({m_play_indicator_1->Render(), m_play_indicator_2->Render(), m_play_indicator_3->Render()});

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