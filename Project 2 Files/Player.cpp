// Takoda Jennings, CS-499, 9/18/25

#include "Player.h"
#include <algorithm>
#include <iostream>
#include <cassert>

// ----------------------- Constructors & Setup -----------------------------
Player::Player(std::string name, std::size_t startingDice, std::mt19937* rng)
    : name_(std::move(name)), active_(true), rng_(rng)
{
    if (name_.empty()) name_ = "Anonymous";     // Defense to avoid empty names
    resetDice(startingDice);                    // Initializes vector size
}

void Player::setRng(std::mt19937* rng) noexcept {
    rng_ = rng;
}

// ----------------------------- Name --------------------------------
void Player::setName(const std::string& playerName) {
    // Defense to keep a reasonable default if an empty input is somehow provided
    name_ = playerName.empty() ? "Anonymous" : playerName;
}

const std::string& Player::getName() const noexcept {
    return name_;
}

// ----------------------------- Dice --------------------------------
void Player::resetDice(std::size_t count) {
    dice_.assign(count, DICE_MIN); // placeholder values; actual values are set in rollDice()
}

void Player::rollDice() {
    // Defense so that RNG is provided from the outside (seeded once at program start)
    if (!rng_) {
        // Fallback for if RNG was not set correctly, creates a static local engine so we can still work.
        static thread_local std::mt19937 fallback{ std::random_device{}() };
        rng_ = &fallback;
    }
    std::uniform_int_distribution<int> dist(DICE_MIN, DICE_MAX);
    for (int& d : dice_) {
        d = dist(*rng_);
        // asserts an invariant explicitly used in debug builds.
        assert(d >= DICE_MIN && d <= DICE_MAX);
    }
}

void Player::removeDie() {
    if (!dice_.empty()) {
        dice_.pop_back(); // actually remove a die (keeps vector and "count" in sync)
    }
}

// -------------------------- Introspection --------------------------
bool Player::isOut() const noexcept {
    return dice_.empty();
}

const std::vector<int>& Player::getDice() const noexcept {
    return dice_;
}

bool Player::isActive() const noexcept {
    return active_;
}

void Player::setActive(bool state) noexcept {
    active_ = state;
}

// --------------------------- UI Helper ------------------------------
// NOTE: Kept for compatibility with existing code that prints directly.
void Player::showDice() const {
    std::cout << name_ << "'s dice: ";
    for (const int d : dice_) {
        std::cout << d << ' ';
    }
    std::cout << '\n';
}