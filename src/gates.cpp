// src/gates.cpp - Logic gate implementations
#include "gates.h"
#include <string>

// ---- AndGate ----

AndGate::AndGate(int in1, int in2, int out)
    : input1(in1), input2(in2), output(out) {}

bool AndGate::evaluate(std::shared_ptr<PlcIO> io) {
    bool result = io->getInput(input1) && io->getInput(input2);
    io->setOutput(output, result);
    return result;
}

std::string AndGate::toString() const {
    return "AND I" + std::to_string(input1) + " & I" + std::to_string(input2)
           + " -> Q" + std::to_string(output);
}

// ---- OrGate ----

OrGate::OrGate(int in1, int in2, int out)
    : input1(in1), input2(in2), output(out) {}

bool OrGate::evaluate(std::shared_ptr<PlcIO> io) {
    bool result = io->getInput(input1) || io->getInput(input2);
    io->setOutput(output, result);
    return result;
}

std::string OrGate::toString() const {
    return "OR  I" + std::to_string(input1) + " | I" + std::to_string(input2)
           + " -> Q" + std::to_string(output);
}

// ---- NotGate ----

NotGate::NotGate(int in, int out)
    : input(in), output(out) {}

bool NotGate::evaluate(std::shared_ptr<PlcIO> io) {
    bool result = !io->getInput(input);
    io->setOutput(output, result);
    return result;
}

std::string NotGate::toString() const {
    return "NOT I" + std::to_string(input) + " -> Q" + std::to_string(output);
}

// ---- RSTrigger ----

RSTrigger::RSTrigger(int set, int reset, int out)
    : setInput(set), resetInput(reset), output(out), state(false) {}

bool RSTrigger::evaluate(std::shared_ptr<PlcIO> io) {
    bool s = io->getInput(setInput);
    bool r = io->getInput(resetInput);

    if (r) {
        state = false;
    } else if (s) {
        state = true;
    }
    // Both false → state unchanged (latch behaviour)

    io->setOutput(output, state);
    return state;
}

std::string RSTrigger::toString() const {
    return "RS  I" + std::to_string(setInput) + "(S) / I"
           + std::to_string(resetInput) + "(R) -> Q" + std::to_string(output);
}
