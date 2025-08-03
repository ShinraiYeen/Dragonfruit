#include "components/now_playing.hpp"

std::string FmtCodeToString(dragonfruit::WavFormatCode code) {
    switch (code) {
        case dragonfruit::WavFormatCode::IEEE_FLOAT:
            return "IEEE Float";
        case dragonfruit::WavFormatCode::PCM:
            return "PCM";
        default:
            return "---";
    };
}

Element NowPlayingBase::OnRender() {
    std::shared_ptr<dragonfruit::Sound> song = player_.GetCurrentSong();

    // If the song doesn't have a name (metadata not found) revert to the file name
    std::string song_name =
        song->Name().empty() ? player_.GetSongQueue()[player_.GetCurrentSongIdx()].filename().string() : song->Name();
    return vbox({
        filler(),
        paragraph(song_name) | hcenter,
        paragraph(song->Artist()) | hcenter,
        paragraph(song->Album()) | hcenter,
        text(""),
        paragraph(std::format("WAV | {} {}-bit | {} Hz", FmtCodeToString(song->Format()), song->BitDepth(),
                              song->SampleRate())) |
            hcenter,
        filler(),
    });
}