// Takoda Jennings, CS-499, 9/18/25

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <limits>
#include <cassert>

#include "Player.h"
#include "Constants.h"

// Simple structure for a bid
struct Bid {
    int quantity = 0; // 0 means "no bid yet"
    int face = 0;     // in [DICE_MIN..DICE_MAX] when quantity > 0
};

// Test-visible rule helper
bool IsBidStrictlyGreater(const Bid& prev, const Bid& next) noexcept;

class Game; // forward declaration

// ---------------------- State Interface (Structure) ------------------------------
class GameState {
public:
    virtual ~GameState() = default;
    // handle() encapsulates the behavior for one step of the state's logic.
    // Is also defensive so as not to throw; as well as validating inputs before state transitions.
    virtual void handle(Game& game) = 0;
    virtual const char* name() const = 0;
};

// --------------------------- Game Context ----------------------------------------
// Responsibilities:
// - Owns current state (unique_ptr for RAII; so no leaks).
// - Stores shared game data (players, current bid, and active player index).
// - Provides helper methods that are reused by concrete states.
// Structure is a clear separation of I/O from rule helpers.
class Game {
public:
    Game();
    void setState(std::unique_ptr<GameState> s) noexcept;
    void playLoop(); // Loop terminates when state becomes GameOverState

    // ---------------- Shared data (Variables) ------------------------------------
    std::vector<Player> players;  // Player contains name, dice, active flag
    Bid currentBid{};
    std::size_t currentPlayer = 0; // Prefer size_t for any kind of indexing

    // ---------------- Helpers (Structure & Defensive Programming) ----------------
    void rollAll();
    int totalDiceInPlay() const noexcept;
    int countFace(int face) const noexcept;
    std::size_t nextActive(std::size_t from) const noexcept;
    bool onlyOneLeft(std::string& winner) const noexcept;
    void showAllDice() const;
    void displayRulesFromFile(const std::string& filename) const;

    // Defensive: small guarded prompts (Loops & Branches + Defensive Programming)
    static int promptInt(const std::string& prompt, int lo, int hi);
    static bool promptYesNo(const std::string& prompt);

private:
    std::unique_ptr<GameState> state;
};

// ---------------------- Concrete States (Structure) ------------------------------
class BiddingState : public GameState {
public:
    const char* name() const override { return "Bidding"; }
    void handle(Game& game) override;
};

class ChallengingState : public GameState {
public:
    const char* name() const override { return "Challenging"; }
    void handle(Game& game) override;
};

class RevealState : public GameState {
public:
    const char* name() const override { return "Reveal"; }
    void handle(Game& game) override;
};

class GameOverState : public GameState {
public:
    const char* name() const override { return "GameOver"; }
    void handle(Game& game) override;
};