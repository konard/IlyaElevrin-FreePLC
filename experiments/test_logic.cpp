// experiments/test_logic.cpp - Unit tests for core PLC logic
// Build: g++ -std=c++17 -I../include test_logic.cpp ../src/plcio.cpp ../src/gates.cpp ../src/ld_program.cpp ../src/relay_manager.cpp -pthread -o test_logic
#include <iostream>
#include <cassert>
#include "plcio.h"
#include "gates.h"
#include "ld_program.h"
#include "relay_manager.h"

static int passed = 0, failed = 0;

#define CHECK(expr, msg) \
    do { if (expr) { std::cout << "[PASS] " << msg << "\n"; ++passed; } \
         else     { std::cout << "[FAIL] " << msg << "\n"; ++failed; } } while(0)

void testPlcIO() {
    PlcIO io("relay1", 4, 4);
    io.setInput(1, true);
    CHECK(io.getInput(1) == true,  "setInput/getInput channel 1 true");
    CHECK(io.getInput(2) == false, "unset input channel 2 is false");
    io.setOutput(1, true);
    CHECK(io.getOutput(1) == true,  "setOutput/getOutput channel 1 true");
    CHECK(io.getOutput(99) == false, "out-of-range output returns false");
}

void testAndGate() {
    auto io = std::make_shared<PlcIO>("r", 4, 4);
    AndGate g(1, 2, 1);

    io->setInput(1, false); io->setInput(2, false);
    g.evaluate(io);
    CHECK(io->getOutput(1) == false, "AND 0,0 = 0");

    io->setInput(1, true); io->setInput(2, false);
    g.evaluate(io);
    CHECK(io->getOutput(1) == false, "AND 1,0 = 0");

    io->setInput(1, true); io->setInput(2, true);
    g.evaluate(io);
    CHECK(io->getOutput(1) == true, "AND 1,1 = 1");
}

void testOrGate() {
    auto io = std::make_shared<PlcIO>("r", 4, 4);
    OrGate g(1, 2, 1);

    io->setInput(1, false); io->setInput(2, false);
    g.evaluate(io);
    CHECK(io->getOutput(1) == false, "OR 0,0 = 0");

    io->setInput(1, true); io->setInput(2, false);
    g.evaluate(io);
    CHECK(io->getOutput(1) == true, "OR 1,0 = 1");

    io->setInput(1, false); io->setInput(2, true);
    g.evaluate(io);
    CHECK(io->getOutput(1) == true, "OR 0,1 = 1");
}

void testNotGate() {
    auto io = std::make_shared<PlcIO>("r", 4, 4);
    NotGate g(1, 1);

    io->setInput(1, false);
    g.evaluate(io);
    CHECK(io->getOutput(1) == true, "NOT 0 = 1");

    io->setInput(1, true);
    g.evaluate(io);
    CHECK(io->getOutput(1) == false, "NOT 1 = 0");
}

void testRSTrigger() {
    auto io = std::make_shared<PlcIO>("r", 4, 4);
    RSTrigger rs(1, 2, 1);   // set=I1, reset=I2, out=Q1

    // Initial state: no set/reset
    io->setInput(1, false); io->setInput(2, false);
    rs.evaluate(io);
    CHECK(io->getOutput(1) == false, "RS initial state = 0");

    // Set
    io->setInput(1, true); io->setInput(2, false);
    rs.evaluate(io);
    CHECK(io->getOutput(1) == true, "RS after SET = 1");

    // Latch (both off)
    io->setInput(1, false); io->setInput(2, false);
    rs.evaluate(io);
    CHECK(io->getOutput(1) == true, "RS latched = 1");

    // Reset
    io->setInput(1, false); io->setInput(2, true);
    rs.evaluate(io);
    CHECK(io->getOutput(1) == false, "RS after RESET = 0");
}

void testRelayManager() {
    RelayManager mgr;
    CHECK(mgr.getCurrentRelay() != nullptr, "Default relay created");
    CHECK(mgr.hasRelay("test"), "Default relay named 'test'");

    mgr.createRelay("r2", 4, 4);
    CHECK(mgr.hasRelay("r2"), "Create new relay r2");

    bool ok = mgr.selectRelay("r2");
    CHECK(ok, "Select relay r2");
    CHECK(mgr.getCurrentRelay()->name == "r2", "Current relay is r2");

    bool notok = mgr.selectRelay("nonexistent");
    CHECK(!notok, "Select nonexistent relay returns false");
}

int main() {
    std::cout << "=== FreePLC Core Logic Tests ===\n\n";
    testPlcIO();
    testAndGate();
    testOrGate();
    testNotGate();
    testRSTrigger();
    testRelayManager();

    std::cout << "\n=== Results: " << passed << " passed, " << failed << " failed ===\n";
    return (failed == 0) ? 0 : 1;
}
