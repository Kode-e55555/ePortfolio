// Takoda Jennings, CS-499, 9/18/25

#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include <random>
#include <cstddef> // for std::size_t

#include "Constants.h"

// ------- Player class — model type for representing a game participant -----------
class Player {
private:
    std::string name_;
    std::vector<int> dice_;   // invariant: all values in [DICE_MIN, DICE_MAX]
    bool active_ = true;
    // RNG is injected to avoid global state and to enable deterministic tests
    std::mt19937* rng_ = nullptr;

public:
    // Construct a player with a name (defaults to "Anonymous"), initial dice count,
    // and optional RNG reference for rolling.
    explicit Player(std::string name = "Anonymous",
        std::size_t startingDice = DEFAULT_DICE,
        std::mt19937* rng = nullptr);

    // Set or update the RNG to be used for rolls
    void setRng(std::mt19937* rng) noexcept;

    // Name accessors
    void setName(const std::string& playerName);
    const std::string& getName() const noexcept;

    // Dice lifecycle
    void resetDice(std::size_t count = DEFAULT_DICE);   // clears and sets new size if unrolled
    void rollDice();                                    // rolls current dice with RNG
    void removeDie();                                   // removes one die if available

    // Introspection
    bool isOut() const noexcept;                        // true if no dice remain
    const std::vector<int>& getDice() const noexcept;   // read-only view of dice
    bool isActive() const noexcept;
    void setActive(bool state) noexcept;

    // Optional UI helper (kept for minimal code churn).
    void showDice() const;
};

#endif