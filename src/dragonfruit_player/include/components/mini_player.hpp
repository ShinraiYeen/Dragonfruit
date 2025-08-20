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
    Player& m_player;
    Component m_play_indicator_1;
    Component m_play_indicator_2;
    Component m_play_indicator_3;
};

inline Component MiniPlayer(Player& player) { return Make<MiniPlayerBase>(player); }