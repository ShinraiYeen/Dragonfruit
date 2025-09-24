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
        // paragraph(std::format("WAV | {} {}-bit | {} Hz", FmtCodeToString(parser->Format()), par->BitDepth(),
        //                       parser->SampleRate())) |
        //     hcenter,
        filler(),
    });
}
