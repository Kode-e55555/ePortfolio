// Takoda Jennings, CS-499, 9/18/25

#include "Game.h"
#include <fstream>

// --------------------- Small input helpers (Defensive) -----------------------
// Defensive guard for invalid inputs, clear failbit, discard lines, and to enforce bounds.
int Game::promptInt(const std::string& prompt, int lo, int hi) {
    while (true) {
        std::cout << prompt;
        int x;
        if (std::cin >> x && x >= lo && x <= hi) return x;
        std::cout << "Invalid input. Please enter " << lo << ".." << hi << ".\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

bool Game::promptYesNo(const std::string& prompt) {
    while (true) {
        std::cout << prompt << " (y/n): ";
        char c;
        if (std::cin >> c) {
            if (c=='y' || c=='Y') return true;
            if (c=='n' || c=='N') return false;
        }
        std::cout << "Please enter 'y' or 'n'.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

// ----------------------------- Game (Structure) ------------------------------
Game::Game() {
    // Displays rules
    if (promptYesNo("View rules?")) displayRulesFromFile("liars_dice_rules.txt");

    // Create players with validated input 
    const int n = promptInt("Enter number of players: ", MIN_PLAYERS, MAX_PLAYERS);
    players.resize(static_cast<std::size_t>(n));
    for (std::size_t i = 0; i < players.size(); ++i) {
        std::cout << "Enter name for Player " << (i + 1) << ": ";
        std::string name;
        while (true) {
            std::cout << "Enter name for Player " << (i + 1) << ": ";
            if (std::getline(std::cin >> std::ws, name) && !name.empty()) {
                break; // Got a non-empty name (allows spaces)
            }
            std::cout << "Please enter a non-empty name.\n";
        }
        players[i].setName(name);
        players[i].setActive(true);
        players[i].rollDice(); 
    }
    currentPlayer = 0; // Variables to initialize indexes just prior to loop use
    setState(std::make_unique<BiddingState>());
}

void Game::setState(std::unique_ptr<GameState> s) noexcept {
    state = std::move(s); // RAII: unique ownership, so that there are no leaks
}

// Loop termination is obvious and achievable: ends when state is GameOverState.
void Game::playLoop() {
    while (state && dynamic_cast<GameOverState*>(state.get()) == nullptr) {
        state->handle(*this);
    }
}

void Game::rollAll() {
    for (auto& p : players) if (p.isActive()) p.rollDice();
}

int Game::totalDiceInPlay() const noexcept {
    int sum = 0;
    for (const auto& p : players) if (p.isActive()) sum += static_cast<int>(p.getDice().size());
    return sum;
}

int Game::countFace(int face) const noexcept {
    int cnt = 0;
    for (const auto& p : players) if (p.isActive()) {
        for (int d : p.getDice()) if (d == face) ++cnt;
    }
    return cnt;
}

// Return next active player index 
std::size_t Game::nextActive(std::size_t from) const noexcept {
    const std::size_t n = players.size();
    for (std::size_t step = 1; step <= n; ++step) {
        const std::size_t idx = (from + step) % n;
        if (players[idx].isActive()) return idx;
    }
    return from; // fallback (shouldn't happen if at least one active player exists)
}

bool Game::onlyOneLeft(std::string& winner) const noexcept {
    int active = 0;
    for (const auto& p : players) {
        if (p.isActive()) { ++active; winner = p.getName(); }
    }
    return active == 1;
}

void Game::showAllDice() const {
    for (const auto& p : players) if (p.isActive()) {
        std::cout << p.getName() << ": ";
        p.showDice();
    }
}

void Game::displayRulesFromFile(const std::string& filename) const {
    std::ifstream infile(filename);
    if (!infile) { std::cout << "(Rules file not found)\n"; return; } // Validation to check for file existence
    std::cout << "----- Rules -----\n";
    std::string line;
    while (std::getline(infile, line)) std::cout << line << "\n";
    std::cout << "-----------------\n";
}

// ---------------------------- Rules helpers ----------------------------------
// Single source of truth for bid ordering
static bool strictlyGreater(const Bid& prev, const Bid& next) noexcept {
    if (prev.quantity == 0) return next.quantity > 0 && next.face >= DICE_MIN && next.face <= DICE_MAX;
    if (next.quantity > prev.quantity) return next.face >= DICE_MIN && next.face <= DICE_MAX;
    if (next.quantity == prev.quantity) return next.face > prev.face && next.face <= DICE_MAX;
    return false;
}

// ------------------------------- States --------------------------------------
void BiddingState::handle(Game& game) {
    // Documentation to make it so that state names are printed and to aid in traceability logs.
    std::cout << "\n-- STATE: " << name() << " -- Current: " << game.players[game.currentPlayer].getName() << "\n";
    std::cout << "Current bid: ";
    if (game.currentBid.quantity == 0) std::cout << "(none)\n";
    else std::cout << game.currentBid.quantity << " of " << game.currentBid.face << "\n";

    // Show only the active player's dice (others remain hidden until Reveal)
    {
        const auto& self = game.players[game.currentPlayer];
        std::cout << "Your dice, " << self.getName() << ": ";
        for (int d : self.getDice()) { std::cout << d << ' '; }
        std::cout << '\n';
    }

    const int maxQ = game.totalDiceInPlay(); // Loop-invariant outside inner loops
    const int q = Game::promptInt("Enter quantity: ", 1, maxQ);
    const int f = Game::promptInt("Enter face: ", DICE_MIN, DICE_MAX);

    const Bid next{q, f};
    if (!strictlyGreater(game.currentBid, next)) {
        std::cout << "Invalid bid: must be strictly greater than previous.\n";
        return; // Remain in BiddingState
    }

    game.currentBid = next;
    game.setState(std::make_unique<ChallengingState>());
}

void ChallengingState::handle(Game& game) {
    std::cout << "\n-- STATE: " << name() << " -- Next player decides.\n";
    const std::size_t challenger = game.nextActive(game.currentPlayer);
    std::cout << game.players[challenger].getName() << ", challenge the bid? \n";
    const bool challenge = Game::promptYesNo("Challenge");

    if (challenge) {
        game.setState(std::make_unique<RevealState>());
    } else {
        game.currentPlayer = challenger; // pass turn to challenger
        game.setState(std::make_unique<BiddingState>());
    }
}

void RevealState::handle(Game& game) {
    std::cout << "\n-- STATE: " << name() << " -- Revealing dice.\n";
    game.showAllDice();

    const int actual = game.countFace(game.currentBid.face);
    std::cout << "Actual count of face " << game.currentBid.face << ": " << actual << "\n";

    const std::size_t bidder = game.currentPlayer;
    const std::size_t challenger = game.nextActive(bidder);
    const bool bidTrue = actual >= game.currentBid.quantity;
    const std::size_t loser = bidTrue ? challenger : bidder;

    std::cout << (bidTrue ? "Bid stands. " : "Bid fails. ")
              << game.players[loser].getName() << " loses a die.\n";

    game.players[loser].removeDie();  // Defensive to ensure non-negative count
    if (game.players[loser].isOut()) {
        std::cout << game.players[loser].getName() << " is out!\n";
        game.players[loser].setActive(false);
    }

    std::string winner;
    if (game.onlyOneLeft(winner)) {
        std::cout << "\n=== " << winner << " wins the game! ===\n";
        game.setState(std::make_unique<GameOverState>());
        return;  // Termination
    }

    // Prepare for next round
    game.currentBid = Bid{};
    game.rollAll();
    game.currentPlayer = game.nextActive(loser); // Loser starts next round
    game.setState(std::make_unique<BiddingState>());
}

void GameOverState::handle(Game& /*game*/) {
    std::cout << "\nGame Over. Thanks for playing!\n";
}


// Test-visible wrapper around internal strictlyGreater
bool IsBidStrictlyGreater(const Bid& prev, const Bid& next) noexcept {
    return strictlyGreater(prev, next);
}
