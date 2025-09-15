#include "components/equalizer.hpp"

EqualizerBase::EqualizerBase(Player& player) : m_player(player) {
    auto slider_option = SliderOption<double>({.value = &m_volume_slider_val,
                                               .min = 0.0,
                                               .max = 1.0,
                                               .increment = 0.25,
                                               .direction = Direction::Right,
                                               .color_active = Color::CornflowerBlue,
                                               .color_inactive = Color::CornflowerBlue,
                                               .on_change = [&]() { m_player.SetVolume(m_volume_slider_val); }});

    m_volume_slider_val = m_player.GetVolume();
    m_volume_slider = Slider(slider_option);
    Add(m_volume_slider);
}

Element EqualizerBase::OnRender() {
    return vbox({
        hbox({
            text(std::format("Volume ({:.2f}) [", m_volume_slider_val)),
            m_volume_slider->Render() | bgcolor(Color::GrayDark),
            text("]"),
        }),
    });
}