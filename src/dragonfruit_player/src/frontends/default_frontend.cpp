#include "frontends/default_frontend.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

#include "components/mini_player.hpp"
#include "components/now_playing.hpp"
#include "components/song_queue.hpp"

void DefaultFrontend::Start() {
    using namespace ftxui;

    // Immediately begin playing the first song
    player_.Play(0);
    player_.SetVolume(1.0);

    // Construct sub components
    auto now_playing = NowPlaying(player_);
    auto song_queue = SongQueue(player_);
    auto mini_player = MiniPlayer(player_);

    // Construct the main menu
    std::vector<Component> screens = {now_playing, song_queue};
    int main_menu_idx;
    const std::vector<std::string> menu_options = {"Now Playing", "Queue"};
    auto menu = Menu(menu_options, &main_menu_idx, MenuOption::HorizontalAnimated());

    // Construct the component layout to pass into the renderer
    auto layout = Container::Vertical({
        menu,
        now_playing,
        song_queue,
        mini_player,
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
            player_.Pause(!player_.IsPaused());
            return true;
        } else if (event == Event::Escape || event == Event::Character('q')) {
            screen.Exit();
            return true;
        } else if (event == Event::ArrowRight) {
            player_.PlayRelative(1);
            return true;
        } else if (event == Event::ArrowLeft) {
            player_.PlayRelative(-1);
            return true;
        } else if (event == Event::Character(",")) {
            player_.Seek(-5.0);
            return true;
        } else if (event == Event::Character(".")) {
            player_.Seek(5.0);
            return true;
        } else if (event == Event::Character("s")) {
            player_.Shuffle();
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

        if (player_.IsFinished()) { player_.PlayRelative(1); }
    }
}