// Takoda Jennings, CS-499, 9/18/25

#include <ctime>
#include <cstdlib>
#include "Game.h"

int main() {
    // Seed RNG once: avoids reseeding per roll and ensures fair distribution
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    Game game;
    game.playLoop();
    return 0;
}
