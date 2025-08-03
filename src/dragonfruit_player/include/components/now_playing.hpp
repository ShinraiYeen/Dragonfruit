#pragma once

#include <dragonfruit_engine/sound.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "player.hpp"

using namespace ftxui;

class NowPlayingBase : public ComponentBase {
   public:
    NowPlayingBase(Player& player) : player_(player) {}

    Element OnRender() override;

   private:
    Player& player_;
};

inline Component NowPlaying(Player& player) { return Make<NowPlayingBase>(player); }