#include "components/now_playing.hpp"

Element NowPlayingBase::OnRender() {
    const dragonfruit::Metadata& metadata = m_player.GetCurMetadata();

    // If the song doesn't have a name (metadata not found) revert to the file name
    std::string song_name = metadata.title == ""
                                ? m_player.GetSongQueue()[m_player.GetCurrentSongIdx()].filename().string()
                                : metadata.title;
    return vbox({
        filler(),
        paragraph(song_name) | hcenter,
        paragraph(metadata.artist) | hcenter,
        paragraph(metadata.album) | hcenter,
        text(""),
        filler(),
    });
}
