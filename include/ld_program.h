// include/ld_program.h - LDProgram: container for logic elements with run/stop
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <atomic>
#include "logic_element.h"
#include "plcio.h"

class LDProgram {
private:
    std::vector<std::shared_ptr<LogicElement>> elements;
    std::shared_ptr<PlcIO> io;
    std::string programName;
    std::atomic<bool> running;

public:
    LDProgram(std::shared_ptr<PlcIO> plcIO, const std::string& name);

    // Non-copyable due to atomic member
    LDProgram(const LDProgram&) = delete;
    LDProgram& operator=(const LDProgram&) = delete;

    void addElement(std::shared_ptr<LogicElement> element);
    void removeElement(int index);
    void clearElements();

    void execute();
    void run();
    void stop();
    bool isRunning() const;

    const std::vector<std::shared_ptr<LogicElement>>& getElements() const;
    std::shared_ptr<PlcIO> getIO() const;
    const std::string& getName() const;
};
