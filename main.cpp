// main.cpp - FreePLC Program for Programmable Relay Simulation (FIXED)
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <algorithm>
#include <thread>
#include <chrono>  // Исправлено: было chronio, теперь chrono
#include <atomic>

// GUI Libraries (using cross-platform approach)
#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

// ==================== Core Logic Classes ====================

class PlcIO {
public:
    std::map<int, bool> inputs;
    std::map<int, bool> outputs;
    std::string name;
    
    PlcIO(const std::string& relayName, int numInputs, int numOutputs) 
        : name(relayName) {
        // Initialize inputs
        for (int i = 1; i <= numInputs; i++) {
            inputs[i] = false;
        }
        // Initialize outputs
        for (int i = 1; i <= numOutputs; i++) {
            outputs[i] = false;
        }
    }
    
    void setInput(int channel, bool value) {
        if (inputs.find(channel) != inputs.end()) {
            inputs[channel] = value;
        }
    }
    
    bool getInput(int channel) {
        return inputs.count(channel) ? inputs[channel] : false;
    }
    
    void setOutput(int channel, bool value) {
        if (outputs.find(channel) != outputs.end()) {
            outputs[channel] = value;
        }
    }
    
    bool getOutput(int channel) {
        return outputs.count(channel) ? outputs[channel] : false;
    }
};

// Base class for logic elements
class LogicElement {
public:
    virtual bool evaluate(std::shared_ptr<PlcIO> io) = 0;
    virtual std::string toString() = 0;  // Pure virtual function
    virtual ~LogicElement() = default;
};

// AND Gate
class AndGate : public LogicElement {
private:
    int input1;
    int input2;
    int output;
    
public:
    AndGate(int in1, int in2, int out) : input1(in1), input2(in2), output(out) {}
    
    bool evaluate(std::shared_ptr<PlcIO> io) override {
        bool result = io->getInput(input1) && io->getInput(input2);
        io->setOutput(output, result);
        return result;
    }
    
    std::string toString() override {
        return "AND I" + std::to_string(input1) + " & I" + std::to_string(input2) + " -> Q" + std::to_string(output);
    }
};

// OR Gate
class OrGate : public LogicElement {
private:
    int input1;
    int input2;
    int output;
    
public:
    OrGate(int in1, int in2, int out) : input1(in1), input2(in2), output(out) {}
    
    bool evaluate(std::shared_ptr<PlcIO> io) override {
        bool result = io->getInput(input1) || io->getInput(input2);
        io->setOutput(output, result);
        return result;
    }
    
    std::string toString() override {
        return "OR I" + std::to_string(input1) + " | I" + std::to_string(input2) + " -> Q" + std::to_string(output);
    }
};

// NOT Gate (Inverter)
class NotGate : public LogicElement {
private:
    int input;
    int output;
    
public:
    NotGate(int in, int out) : input(in), output(out) {}
    
    bool evaluate(std::shared_ptr<PlcIO> io) override {
        bool result = !io->getInput(input);
        io->setOutput(output, result);
        return result;
    }
    
    std::string toString() override {
        return "NOT I" + std::to_string(input) + " -> Q" + std::to_string(output);
    }
};

// RS Trigger (Flip-Flop)
class RSTrigger : public LogicElement {
private:
    int setInput;
    int resetInput;
    int output;
    bool state;
    
public:
    RSTrigger(int set, int reset, int out) : setInput(set), resetInput(reset), output(out), state(false) {}
    
    bool evaluate(std::shared_ptr<PlcIO> io) override {
        bool set = io->getInput(setInput);
        bool reset = io->getInput(resetInput);
        
        if (reset) {
            state = false;
        } else if (set) {
            state = true;
        }
        // If both set and reset are false, state remains unchanged
        
        io->setOutput(output, state);
        return state;
    }
    
    std::string toString() override {
        return "RS I" + std::to_string(setInput) + "(S) / I" + std::to_string(resetInput) + "(R) -> Q" + std::to_string(output);
    }
};

// LD Program Class
class LDProgram {
private:
    std::vector<std::shared_ptr<LogicElement>> elements;
    std::shared_ptr<PlcIO> io;
    std::string programName;
    std::atomic<bool> running;
    
public:
    LDProgram(std::shared_ptr<PlcIO> plcIO, const std::string& name) 
        : io(plcIO), programName(name), running(false) {}
    
    void addElement(std::shared_ptr<LogicElement> element) {
        elements.push_back(element);
    }
    
    void removeElement(int index) {
        if (index >= 0 && index < static_cast<int>(elements.size())) {
            elements.erase(elements.begin() + index);
        }
    }
    
    void execute() {
        for (auto& element : elements) {
            element->evaluate(io);
        }
    }
    
    void run() {
        running = true;
        std::cout << "Program " << programName << " started. Press 'q' to stop.\n";
        
        while (running) {
            execute();
            displayIO();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    
    void stop() {
        running = false;
    }
    
    void displayIO() {
        std::cout << "\n=== I/O Status ===\n";
        std::cout << "Inputs:  ";
        for (const auto& input : io->inputs) {
            std::cout << "I" << input.first << "=" << (input.second ? "1" : "0") << " ";
        }
        std::cout << "\nOutputs: ";
        for (const auto& output : io->outputs) {
            std::cout << "Q" << output.first << "=" << (output.second ? "1" : "0") << " ";
        }
        std::cout << "\n==================\n";
    }
    
    void listProgram() {
        std::cout << "\n=== Program: " << programName << " ===\n";
        for (size_t i = 0; i < elements.size(); i++) {
            std::cout << i+1 << ". " << elements[i]->toString() << "\n";
        }
        if (elements.empty()) {
            std::cout << "Program is empty.\n";
        }
    }
};

// ==================== GUI Classes ====================

class Menu {
private:
    std::vector<std::string> options;
    std::string title;
    
public:
    Menu(const std::string& menuTitle) : title(menuTitle) {}
    
    void addOption(const std::string& option) {
        options.push_back(option);
    }
    
    int display() {
        clearScreen();
        std::cout << "\n=== " << title << " ===\n\n";
        for (size_t i = 0; i < options.size(); i++) {
            std::cout << i+1 << ". " << options[i] << "\n";
        }
        std::cout << "\n0. Exit\n";
        std::cout << "Choice: ";
        
        int choice;
        std::cin >> choice;
        return choice;
    }
    
    void clearScreen() {
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif
    }
    
    void waitForKey() {
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore();
        std::cin.get();
    }
};

class RelayManager {
private:
    std::map<std::string, std::shared_ptr<PlcIO>> relays;
    std::map<std::string, std::shared_ptr<LDProgram>> programs;
    std::shared_ptr<PlcIO> currentRelay;
    std::shared_ptr<LDProgram> currentProgram;
    
public:
    RelayManager() {
        // Create default test relay
        createRelay("test", 6, 6);
    }
    
    void createRelay(const std::string& name, int inputs, int outputs) {
        auto relay = std::make_shared<PlcIO>(name, inputs, outputs);
        relays[name] = relay;
        auto program = std::make_shared<LDProgram>(relay, name + "_program");
        programs[name] = program;
    }
    
    bool selectRelay(const std::string& name) {
        if (relays.find(name) != relays.end()) {
            currentRelay = relays[name];
            currentProgram = programs[name];
            return true;
        }
        return false;
    }
    
    std::shared_ptr<PlcIO> getCurrentRelay() { return currentRelay; }
    std::shared_ptr<LDProgram> getCurrentProgram() { return currentProgram; }
    
    void listRelays() {
        std::cout << "\nAvailable Relays:\n";
        for (const auto& relay : relays) {
            std::cout << "- " << relay.first 
                      << " (I:" << relay.second->inputs.size() 
                      << "/O:" << relay.second->outputs.size() << ")\n";
        }
    }
};

// ==================== Main Application ====================

class FreePLCApp {
private:
    RelayManager manager;
    std::shared_ptr<PlcIO> currentRelay;
    std::shared_ptr<LDProgram> currentProgram;
    
public:
    FreePLCApp() {
        manager.selectRelay("test");
        currentRelay = manager.getCurrentRelay();
        currentProgram = manager.getCurrentProgram();
    }
    
    void run() {
        while (true) {
            Menu mainMenu("FreePLC - Main Menu");
            mainMenu.addOption("Select Relay");
            mainMenu.addOption("Program LD");
            mainMenu.addOption("Run Program");
            mainMenu.addOption("Manual I/O Control");
            mainMenu.addOption("View I/O Status");
            
            int choice = mainMenu.display();
            
            switch (choice) {
                case 1: selectRelayMenu(); break;
                case 2: programLDMenu(); break;
                case 3: runProgram(); break;
                case 4: manualIOControl(); break;
                case 5: viewIOStatus(); break;
                case 0: 
                    std::cout << "Exiting FreePLC...\n";
                    return;
                default:
                    std::cout << "Invalid choice!\n";
                    mainMenu.waitForKey();
            }
        }
    }
    
    void selectRelayMenu() {
        manager.listRelays();
        
        std::string relayName;
        std::cout << "\nEnter relay name (or 'new' to create): ";
        std::cin >> relayName;
        
        if (relayName == "new") {
            std::string name;
            int inputs, outputs;
            std::cout << "Enter relay name: ";
            std::cin >> name;
            std::cout << "Number of inputs: ";
            std::cin >> inputs;
            std::cout << "Number of outputs: ";
            std::cin >> outputs;
            
            manager.createRelay(name, inputs, outputs);
            manager.selectRelay(name);
            std::cout << "Relay " << name << " created and selected.\n";
        } else {
            if (manager.selectRelay(relayName)) {
                currentRelay = manager.getCurrentRelay();
                currentProgram = manager.getCurrentProgram();
                std::cout << "Relay " << relayName << " selected.\n";
            } else {
                std::cout << "Relay not found!\n";
            }
        }
        
        Menu("").waitForKey();
    }
    
    void programLDMenu() {
        if (!currentRelay) {
            std::cout << "No relay selected!\n";
            Menu("").waitForKey();
            return;
        }
        
        while (true) {
            Menu progMenu("LD Programming - " + currentRelay->name);
            progMenu.addOption("List Program");
            progMenu.addOption("Add AND Gate");
            progMenu.addOption("Add OR Gate");
            progMenu.addOption("Add NOT Gate");
            progMenu.addOption("Add RS Trigger");
            progMenu.addOption("Remove Element");
            progMenu.addOption("Clear Program");
            
            int choice = progMenu.display();
            
            switch (choice) {
                case 1: 
                    currentProgram->listProgram();
                    progMenu.waitForKey();
                    break;
                    
                case 2: {
                    int in1, in2, out;
                    std::cout << "Enter input 1 channel: ";
                    std::cin >> in1;
                    std::cout << "Enter input 2 channel: ";
                    std::cin >> in2;
                    std::cout << "Enter output channel: ";
                    std::cin >> out;
                    
                    currentProgram->addElement(std::make_shared<AndGate>(in1, in2, out));
                    std::cout << "AND gate added.\n";
                    progMenu.waitForKey();
                    break;
                }
                
                case 3: {
                    int in1, in2, out;
                    std::cout << "Enter input 1 channel: ";
                    std::cin >> in1;
                    std::cout << "Enter input 2 channel: ";
                    std::cin >> in2;
                    std::cout << "Enter output channel: ";
                    std::cin >> out;
                    
                    currentProgram->addElement(std::make_shared<OrGate>(in1, in2, out));
                    std::cout << "OR gate added.\n";
                    progMenu.waitForKey();
                    break;
                }
                
                case 4: {
                    int in, out;
                    std::cout << "Enter input channel: ";
                    std::cin >> in;
                    std::cout << "Enter output channel: ";
                    std::cin >> out;
                    
                    currentProgram->addElement(std::make_shared<NotGate>(in, out));
                    std::cout << "NOT gate added.\n";
                    progMenu.waitForKey();
                    break;
                }
                
                case 5: {
                    int set, reset, out;
                    std::cout << "Enter SET input channel: ";
                    std::cin >> set;
                    std::cout << "Enter RESET input channel: ";
                    std::cin >> reset;
                    std::cout << "Enter output channel: ";
                    std::cin >> out;
                    
                    currentProgram->addElement(std::make_shared<RSTrigger>(set, reset, out));
                    std::cout << "RS Trigger added.\n";
                    progMenu.waitForKey();
                    break;
                }
                
                case 6: {
                    currentProgram->listProgram();
                    int index;
                    std::cout << "Enter element number to remove: ";
                    std::cin >> index;
                    currentProgram->removeElement(index - 1);
                    std::cout << "Element removed.\n";
                    progMenu.waitForKey();
                    break;
                }
                
                case 7: {
                    std::cout << "Are you sure? (y/n): ";
                    char confirm;
                    std::cin >> confirm;
                    if (confirm == 'y' || confirm == 'Y') {
                        // Clear program by creating new one
                        currentProgram = std::make_shared<LDProgram>(currentRelay, currentRelay->name + "_program");
                        std::cout << "Program cleared.\n";
                    }
                    progMenu.waitForKey();
                    break;
                }
                
                case 0:
                    return;
                    
                default:
                    std::cout << "Invalid choice!\n";
                    progMenu.waitForKey();
            }
        }
    }
    
    void runProgram() {
        if (!currentProgram) {
            std::cout << "No program loaded!\n";
            Menu("").waitForKey();
            return;
        }
        
        std::cout << "Starting program execution. Press 'q' to stop.\n";
        std::cout << "You can set inputs in another terminal by running manual I/O control.\n";
        Menu("").waitForKey();
        
        // Run program in a separate thread to allow for input simulation
        std::thread programThread([this]() {
            currentProgram->run();
        });
        
        // Wait for user to press 'q'
        char ch;
        do {
            std::cin >> ch;
        } while (ch != 'q');
        
        currentProgram->stop();
        if (programThread.joinable()) {
            programThread.join();
        }
    }
    
    void manualIOControl() {
        if (!currentRelay) {
            std::cout << "No relay selected!\n";
            Menu("").waitForKey();
            return;
        }
        
        while (true) {
            Menu ioMenu("Manual I/O Control");
            ioMenu.addOption("Set Input");
            ioMenu.addOption("Reset Input");
            ioMenu.addOption("View All I/O");
            
            int choice = ioMenu.display();
            
            switch (choice) {
                case 1: {
                    int channel;
                    std::cout << "Enter input channel (1-" << currentRelay->inputs.size() << "): ";
                    std::cin >> channel;
                    currentRelay->setInput(channel, true);
                    std::cout << "Input I" << channel << " set to 1.\n";
                    ioMenu.waitForKey();
                    break;
                }
                
                case 2: {
                    int channel;
                    std::cout << "Enter input channel (1-" << currentRelay->inputs.size() << "): ";
                    std::cin >> channel;
                    currentRelay->setInput(channel, false);
                    std::cout << "Input I" << channel << " set to 0.\n";
                    ioMenu.waitForKey();
                    break;
                }
                
                case 3:
                    currentProgram->displayIO();
                    ioMenu.waitForKey();
                    break;
                    
                case 0:
                    return;
                    
                default:
                    std::cout << "Invalid choice!\n";
                    ioMenu.waitForKey();
            }
        }
    }
    
    void viewIOStatus() {
        if (!currentRelay) {
            std::cout << "No relay selected!\n";
        } else {
            currentProgram->displayIO();
        }
        Menu("").waitForKey();
    }
};

// ==================== Main Function ====================

int main() {
    std::cout << "====================================\n";
    std::cout << "      FreePLC v1.0\n";
    std::cout << "  Programmable Relay Simulator\n";
    std::cout << "====================================\n\n";
    
    FreePLCApp app;
    app.run();
    
    return 0;
}