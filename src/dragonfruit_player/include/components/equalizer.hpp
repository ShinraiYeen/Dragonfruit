#pragma once

#include <dragonfruit_engine/audio_engine.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

#include "player.hpp"

using namespace ftxui;

class EqualizerBase : public ComponentBase {
   public:
    EqualizerBase(Player& player);

    Element OnRender() override;

   private:
    Player& m_player;
    Component m_volume_slider;
    double m_volume_slider_val = 0;
};

inline Component Equalizer(Player& player) { return Make<EqualizerBase>(player); }