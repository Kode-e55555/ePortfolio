// Takoda Jennings, CS-499, 9/18/25

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>

// ------------------------------- Constants (to avoid magic numbers) -----------------------------------------
constexpr int DICE_MIN = 1;               // Symbolic constants so that there are no magic numbers
constexpr int DICE_MAX = 6;               // Used in bid validation and dice display
constexpr std::size_t DEFAULT_DICE = 5;
constexpr int MIN_PLAYERS = 2;            // Replaces previously used literals with a named constant
constexpr int MAX_PLAYERS = 6;           // Same as above and is set to 6 for now (could be made malleable in the future for house rules)

#endif