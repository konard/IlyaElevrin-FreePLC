// include/plcio.h - PlcIO class representing a PLC's I/O channels
#pragma once

#include <map>
#include <string>

class PlcIO {
public:
    std::map<int, bool> inputs;
    std::map<int, bool> outputs;
    std::string name;

    PlcIO(const std::string& relayName, int numInputs, int numOutputs);

    void setInput(int channel, bool value);
    bool getInput(int channel) const;

    void setOutput(int channel, bool value);
    bool getOutput(int channel) const;
};
