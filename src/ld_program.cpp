// src/ld_program.cpp - LDProgram implementation
#include "ld_program.h"
#include <iostream>
#include <thread>
#include <chrono>

LDProgram::LDProgram(std::shared_ptr<PlcIO> plcIO, const std::string& name)
    : io(plcIO), programName(name), running(false) {}

void LDProgram::addElement(std::shared_ptr<LogicElement> element) {
    elements.push_back(element);
}

void LDProgram::removeElement(int index) {
    if (index >= 0 && index < static_cast<int>(elements.size())) {
        elements.erase(elements.begin() + index);
    }
}

void LDProgram::clearElements() {
    elements.clear();
}

void LDProgram::execute() {
    for (auto& element : elements) {
        element->evaluate(io);
    }
}

void LDProgram::run() {
    running = true;
    while (running) {
        execute();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void LDProgram::stop() {
    running = false;
}

bool LDProgram::isRunning() const {
    return running.load();
}

const std::vector<std::shared_ptr<LogicElement>>& LDProgram::getElements() const {
    return elements;
}

std::shared_ptr<PlcIO> LDProgram::getIO() const {
    return io;
}

const std::string& LDProgram::getName() const {
    return programName;
}
