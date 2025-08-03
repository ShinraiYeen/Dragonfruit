#pragma once

#include <dragonfruit_engine/audio_engine.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "player.hpp"

using namespace ftxui;

class MiniPlayerBase : public ComponentBase {
   public:
    MiniPlayerBase(Player& player);

    Element OnRender() override;

   private:
    Player& player_;
    Component play_indicator_1_;
    Component play_indicator_2_;
    Component play_indicator_3_;
};

inline Component MiniPlayer(Player& player) { return Make<MiniPlayerBase>(player); }