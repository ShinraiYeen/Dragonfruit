#include "frontends/default_frontend.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include "components/equalizer.hpp"
#include "components/mini_player.hpp"
#include "components/now_playing.hpp"
#include "components/song_queue.hpp"

void DefaultFrontend::Start() {
    using namespace ftxui;

    // Immediately begin playing the first song
    m_player.Play(0);
    m_player.SetVolume(1.0);

    // Construct sub components
    auto now_playing = NowPlaying(m_player);
    auto song_queue = SongQueue(m_player);
    auto mini_player = MiniPlayer(m_player);
    auto equalizer = Equalizer(m_player);

    // Construct the main menu
    std::vector<Component> screens = {now_playing, song_queue, equalizer};
    int main_menu_idx = 0;
    const std::vector<std::string> menu_options = {"Now Playing", "Queue", "Equalizer"};
    auto menu = Menu(menu_options, &main_menu_idx, MenuOption::HorizontalAnimated());

    // Construct the component layout to pass into the renderer
    auto layout = Container::Vertical({
        menu,
        now_playing,
        song_queue,
        mini_player,
        equalizer,
    });

    auto screen = ScreenInteractive::Fullscreen();

    // Define the FTXUI renderer, which is responsible for rendering all of the components and the main interface
    auto component = Renderer(layout, [&] {
        return window(text("DragonfruitPlayer"), vbox({
                                                     menu->Render(),
                                                     //   filler(),
                                                     screens[main_menu_idx]->Render() | flex,
                                                     filler(),
                                                     separatorEmpty(),
                                                     mini_player->Render(),
                                                 }));
    });

    component |= CatchEvent([&](Event event) -> bool {
        if (event == Event::Character(' ')) {
            m_player.Pause(!m_player.IsPaused());
            return true;
        } else if (event == Event::Escape || event == Event::Character('q')) {
            screen.Exit();
            return true;
        } else if (event == Event::ArrowRight) {
            m_player.PlayRelative(1);
            return true;
        } else if (event == Event::ArrowLeft) {
            m_player.PlayRelative(-1);
            return true;
        } else if (event == Event::Character(",")) {
            m_player.Seek(-5.0);
            return true;
        } else if (event == Event::Character(".")) {
            m_player.Seek(5.0);
            return true;
        } else if (event == Event::Character("s")) {
            m_player.Shuffle();
            return true;
        }
        return false;
    });

    Loop loop(&screen, component);

    // Begin main frontend loop
    while (!loop.HasQuitted()) {
        loop.RunOnce();
        screen.RequestAnimationFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (m_player.IsFinished()) {
            m_player.PlayRelative(1);
        }
    }
}