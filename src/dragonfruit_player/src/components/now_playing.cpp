#include "components/now_playing.hpp"

Element NowPlayingBase::OnRender() {
    auto parser = m_player.GetParser();

    // If the song doesn't have a name (metadata not found) revert to the file name
    std::string song_name = parser->Name() == ""
                                ? m_player.GetSongQueue()[m_player.GetCurrentSongIdx()].filename().string()
                                : parser->Name();
    return vbox({
        filler(),
        paragraph(song_name) | hcenter,
        paragraph(parser->Artist()) | hcenter,
        paragraph(parser->Album()) | hcenter,
        text(""),
        filler(),
    });
}
