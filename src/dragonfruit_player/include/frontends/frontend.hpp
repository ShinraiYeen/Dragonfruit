#pragma once

#include "player.hpp"

/**
 * @brief Abstract class to define a frontend.
 *
 */
class Frontend {
   public:
    virtual ~Frontend() = default;

    /**
     * @brief Starts the main frontend loop. Blocks until the frontend exits.
     *
     */
    virtual void Start() = 0;

   protected:
    Frontend(Player& player) : m_player(player) {}
    // A reference to the main Player object is required to make calls to the backend audio engine.
    Player& m_player;
};