// src/plcio.cpp - PlcIO implementation
#include "plcio.h"

PlcIO::PlcIO(const std::string& relayName, int numInputs, int numOutputs)
    : name(relayName)
{
    for (int i = 1; i <= numInputs; i++) {
        inputs[i] = false;
    }
    for (int i = 1; i <= numOutputs; i++) {
        outputs[i] = false;
    }
}

void PlcIO::setInput(int channel, bool value) {
    if (inputs.find(channel) != inputs.end()) {
        inputs[channel] = value;
    }
}

bool PlcIO::getInput(int channel) const {
    auto it = inputs.find(channel);
    return (it != inputs.end()) ? it->second : false;
}

void PlcIO::setOutput(int channel, bool value) {
    if (outputs.find(channel) != outputs.end()) {
        outputs[channel] = value;
    }
}

bool PlcIO::getOutput(int channel) const {
    auto it = outputs.find(channel);
    return (it != outputs.end()) ? it->second : false;
}
