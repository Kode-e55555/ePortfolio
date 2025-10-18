# Liar’s Dice – Enhancement Pseudocode

This document summarizes the newly added / refactored logic in pseudocode.

---

## 1) State Pattern for Game Flow

**Intent:** Separate phases (Bidding → Challenging → Reveal → Game Over) and make transitions explicit.

```
Class Game
    has: currentState (one of: BiddingState, ChallengingState, RevealState, GameOverState)
    has: players (list), activePlayerIndex, lastBid, dicePerPlayer

    function setState(newState)
        currentState = newState

    function playTurn()
        // delegate the rules of this phase to the current state object
        currentState.handle(game=self)

Interface GameState
    function handle(game)

Class BiddingState implements GameState
    function handle(game)
        show active player's dice to only that player
        prompt active player for a bid (quantity + face)
        if bid is invalid (not higher than lastBid OR out of range):
            inform player and re‑prompt within this state
        else
            game.lastBid = entered bid
            game.setState(ChallengingState)

Class ChallengingState implements GameState
    function handle(game)
        announce last bid to the other player(s)
        prompt next player to either "challenge" or "raise"
        if challenge chosen:
            game.setState(RevealState)
        else if valid higher bid provided:
            game.lastBid = new higher bid
            advance active player
            stay in BiddingState for next turn
            game.setState(BiddingState)
        else
            inform player and re‑prompt in this state

Class RevealState implements GameState
    function handle(game)
        reveal all dice from every active player
        count how many dice match the bid face (ones may be wild only if rule enabled)
        if count >= bid.quantity:
            challenger loses one die
        else
            bidder loses one die
        remove a die from the losing player
        eliminate any player with zero dice (set active=false)

        if only one active player remains:
            game.setState(GameOverState)
        else
            set next active player (usually loser starts next round)
            clear lastBid
            re‑roll dice for all active players
            game.setState(BiddingState)

Class GameOverState implements GameState
    function handle(game)
        announce the winner
        end loop
```

---

## 2) Input Validation & Safer Prompts

```
function promptBid()
    loop
        read a full line of text
        parse quantity and face from the line
        if parsing fails OR numbers not within [1..maxDice] and [1..6]:
            tell user what was wrong and continue
        if not strictly higher than lastBid by (quantity, then face):
            explain ordering rule and continue
        return valid bid
```

```
function safeReadName(prompt)
    print prompt
    read entire line (supports spaces and trailing periods)
    trim whitespace
    while line empty:
        print "Name cannot be empty."
        read line again
    return line
```

---

## 3) Visibility Fix: Active Player Can See Their Own Dice

```
function rollAndDisplayForActivePlayer()
    for each player in players:
        roll that player's hidden dice

    privately show only active player's dice (e.g., mask others as '*')
    do NOT echo other players' dice to the console
```

---

## 4) Turn Management Utilities

```
function nextActivePlayerIndex(fromIndex)
    i = (fromIndex + 1) mod number_of_players
    while players[i] is not active:
        i = (i + 1) mod number_of_players
    return i
```

```
function removeDie(player)
    if player has at least one die:
        decrement die count
    if die count == 0:
        mark player inactive
```
