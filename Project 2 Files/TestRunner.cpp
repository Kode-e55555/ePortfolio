// Takoda Jennings, CS-499, 9/18/25

#include <iostream>
#include <random>
#include <vector>
#include "../Constants.h"
#include "../Player.h"
#include "../Game.h"

#define ASSERT_TRUE(x) do { if(!(x)) { std::cerr << __FILE__ << ":" << __LINE__ << ": ASSERT_TRUE(" #x ") failed\n"; return false; } } while(0)
#define ASSERT_EQ(a,b) do { auto _av=(a); auto _bv=(b); if(!(_av==_bv)) { std::cerr << __FILE__ << ":" << __LINE__ << ": ASSERT_EQ failed: " << _av << " != " << _bv << "\n"; return false; } } while(0)
#define TEST(name) bool name()

// ---- Tests ----

// Bid ordering rule
TEST(Test_BidStrictlyGreater) {
    Bid none{0,0};
    Bid q1f2{1,2};
    Bid q1f3{1,3};
    Bid q2f1{2,1};
    ASSERT_TRUE(IsBidStrictlyGreater(none, q1f2));      // first bid allowed
    ASSERT_TRUE(!IsBidStrictlyGreater(q1f2, q1f2));     // not strictly greater
    ASSERT_TRUE(IsBidStrictlyGreater(q1f2, q1f3));      // same qty, higher face
    ASSERT_TRUE(IsBidStrictlyGreater(q1f3, q2f1));      // higher quantity
    ASSERT_TRUE(!IsBidStrictlyGreater(q1f3, Bid{1,1})); // lower face
    // bounds
    ASSERT_TRUE(!IsBidStrictlyGreater(none, Bid{1,0}));
    ASSERT_TRUE(!IsBidStrictlyGreater(none, Bid{1,7}));
    return true;
}

// Player dice lifecycle
TEST(Test_Player_Roll_Remove_Out) {
    std::mt19937 rng(12345); // deterministic
    Player p("Tester", 5, &rng);
    p.rollDice();
    for(int d : p.getDice()) {
        ASSERT_TRUE(d >= DICE_MIN && d <= DICE_MAX);
    }
    // remove all
    for(int i=0;i<5;++i) p.removeDie();
    ASSERT_TRUE(p.isOut());
    // removing when empty should not crash
    p.removeDie();
    ASSERT_TRUE(p.isOut());
    return true;
}

// Reset and re-roll
TEST(Test_Player_Reset_Reroll) {
    std::mt19937 rng(42);
    Player p("Tester", 2, &rng);
    p.resetDice(10);
    ASSERT_EQ(p.getDice().size(), static_cast<size_t>(10));
    p.rollDice();
    for(int d : p.getDice()) {
        ASSERT_TRUE(d >= DICE_MIN && d <= DICE_MAX);
    }
    return true;
}

// ---- Runner ----
int main() {
    int passed = 0, total = 0;
    auto run = [&](bool(*fn)(), const char* name){
        ++total;
        if (fn()) { ++passed; std::cout << "[PASS] " << name << "\n"; }
        else { std::cout << "[FAIL] " << name << "\n"; }
    };

    run(Test_BidStrictlyGreater, "Test_BidStrictlyGreater");
    run(Test_Player_Roll_Remove_Out, "Test_Player_Roll_Remove_Out");
    run(Test_Player_Reset_Reroll, "Test_Player_Reset_Reroll");

    std::cout << passed << "/" << total << " tests passed\n";
    return (passed==total) ? 0 : 1;
}
