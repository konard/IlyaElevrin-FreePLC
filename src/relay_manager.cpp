// src/relay_manager.cpp - RelayManager implementation
#include "relay_manager.h"
#include <algorithm>

RelayManager::RelayManager() {
    createRelay("test", 6, 6);
    selectRelay("test");
}

void RelayManager::createRelay(const std::string& name, int inputs, int outputs) {
    auto relay = std::make_shared<PlcIO>(name, inputs, outputs);
    relays[name] = relay;
    programs[name] = std::make_shared<LDProgram>(relay, name + "_program");
}

bool RelayManager::selectRelay(const std::string& name) {
    auto it = relays.find(name);
    if (it != relays.end()) {
        currentRelay = it->second;
        currentProgram = programs[name];
        return true;
    }
    return false;
}

std::shared_ptr<PlcIO> RelayManager::getCurrentRelay() const {
    return currentRelay;
}

std::shared_ptr<LDProgram> RelayManager::getCurrentProgram() const {
    return currentProgram;
}

std::vector<std::string> RelayManager::getRelayNames() const {
    std::vector<std::string> names;
    for (const auto& kv : relays) {
        names.push_back(kv.first);
    }
    return names;
}

bool RelayManager::hasRelay(const std::string& name) const {
    return relays.find(name) != relays.end();
}
