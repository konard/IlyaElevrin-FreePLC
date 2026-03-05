// include/relay_manager.h - RelayManager: manages multiple PLC relays
#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include "plcio.h"
#include "ld_program.h"

class RelayManager {
private:
    std::map<std::string, std::shared_ptr<PlcIO>> relays;
    std::map<std::string, std::shared_ptr<LDProgram>> programs;
    std::shared_ptr<PlcIO> currentRelay;
    std::shared_ptr<LDProgram> currentProgram;

public:
    RelayManager();

    void createRelay(const std::string& name, int inputs, int outputs);
    bool selectRelay(const std::string& name);

    std::shared_ptr<PlcIO> getCurrentRelay() const;
    std::shared_ptr<LDProgram> getCurrentProgram() const;

    std::vector<std::string> getRelayNames() const;
    bool hasRelay(const std::string& name) const;
};
