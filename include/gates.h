// include/gates.h - Concrete logic gate implementations (AND, OR, NOT, RS Trigger)
#pragma once

#include "logic_element.h"

class AndGate : public LogicElement {
private:
    int input1, input2, output;
public:
    AndGate(int in1, int in2, int out);
    bool evaluate(std::shared_ptr<PlcIO> io) override;
    std::string toString() const override;
};

class OrGate : public LogicElement {
private:
    int input1, input2, output;
public:
    OrGate(int in1, int in2, int out);
    bool evaluate(std::shared_ptr<PlcIO> io) override;
    std::string toString() const override;
};

class NotGate : public LogicElement {
private:
    int input, output;
public:
    NotGate(int in, int out);
    bool evaluate(std::shared_ptr<PlcIO> io) override;
    std::string toString() const override;
};

class RSTrigger : public LogicElement {
private:
    int setInput, resetInput, output;
    bool state;
public:
    RSTrigger(int set, int reset, int out);
    bool evaluate(std::shared_ptr<PlcIO> io) override;
    std::string toString() const override;
};
