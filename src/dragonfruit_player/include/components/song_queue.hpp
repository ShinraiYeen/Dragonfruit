#pragma once

#include <filesystem>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "components/playing_indicator.hpp"
#include "player.hpp"

using namespace ftxui;

class SongQueueBase : public ComponentBase {
   public:
    SongQueueBase(Player& player);

    Element OnRender() override;

   private:
    Player& m_player;
    Component playing_indicator_;
};

inline Component SongQueue(Player& player) { return Make<SongQueueBase>(player); }