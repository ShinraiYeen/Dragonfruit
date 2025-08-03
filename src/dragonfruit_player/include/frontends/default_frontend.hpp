#pragma once

#include "frontends/frontend.hpp"

/**
 * @brief Defines a default frontend for the Dragonfruit player UI.
 *
 */
class DefaultFrontend : public Frontend {
   public:
    DefaultFrontend(Player& player) : Frontend(player) {}

    void Start() override;
};