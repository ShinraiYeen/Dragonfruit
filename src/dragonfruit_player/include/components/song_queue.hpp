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
    Player& player_;
    Component playing_indicator_;
};

static Component SongQueue(Player& player) { return Make<SongQueueBase>(player); }