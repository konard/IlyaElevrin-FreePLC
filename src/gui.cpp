// src/gui.cpp - ncurses-based GUI for FreePLC
#include "gui.h"
#include "gates.h"

#include <ncurses.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <cstring>

// ============================================================
// Layout constants
// ============================================================
static const int TITLE_LINES  = 3;
static const int STATUS_LINES = 2;
static const int IO_COLS      = 30;   // right panel width

// ============================================================
// Constructor / Destructor
// ============================================================

FreePLCGui::FreePLCGui()
    : mainWin(nullptr), titleWin(nullptr),
      menuWin(nullptr), statusWin(nullptr), ioWin(nullptr)
{
    // ncurses init
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    // Colour pairs
    init_pair(1, COLOR_WHITE,  COLOR_BLUE);   // title bar
    init_pair(2, COLOR_BLACK,  COLOR_WHITE);  // normal menu item
    init_pair(3, COLOR_WHITE,  COLOR_CYAN);   // selected menu item
    init_pair(4, COLOR_GREEN,  COLOR_BLACK);  // ON / active
    init_pair(5, COLOR_RED,    COLOR_BLACK);  // OFF / inactive
    init_pair(6, COLOR_YELLOW, COLOR_BLACK);  // highlight / header
    init_pair(7, COLOR_BLACK,  COLOR_GREEN);  // status bar ok
    init_pair(8, COLOR_BLACK,  COLOR_RED);    // status bar error

    getmaxyx(stdscr, screenRows, screenCols);
    initWindows();
}

FreePLCGui::~FreePLCGui() {
    destroyWindows();
    endwin();
}

// ============================================================
// Window management
// ============================================================

void FreePLCGui::initWindows() {
    getmaxyx(stdscr, screenRows, screenCols);

    // Title bar at top
    titleWin  = newwin(TITLE_LINES, screenCols, 0, 0);
    // Status bar at bottom
    statusWin = newwin(STATUS_LINES, screenCols, screenRows - STATUS_LINES, 0);
    // IO panel on the right
    ioWin     = newwin(screenRows - TITLE_LINES - STATUS_LINES,
                       IO_COLS,
                       TITLE_LINES,
                       screenCols - IO_COLS);
    // Main content area
    menuWin   = newwin(screenRows - TITLE_LINES - STATUS_LINES,
                       screenCols - IO_COLS,
                       TITLE_LINES, 0);

    drawTitle();
    drawStatus("Welcome to FreePLC  |  Use arrow keys to navigate, Enter to select");
    drawIOStatus();
}

void FreePLCGui::destroyWindows() {
    if (titleWin)  { delwin(titleWin);  titleWin  = nullptr; }
    if (statusWin) { delwin(statusWin); statusWin = nullptr; }
    if (ioWin)     { delwin(ioWin);     ioWin     = nullptr; }
    if (menuWin)   { delwin(menuWin);   menuWin   = nullptr; }
}

void FreePLCGui::drawTitle() {
    wbkgd(titleWin, COLOR_PAIR(1));
    werase(titleWin);
    int cols = getmaxx(titleWin);
    const char* text = " FreePLC - Programmable Relay Simulator";
    const char* ver  = "v2.0 ";
    wattron(titleWin, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(titleWin, 0, 0, "%*s", cols, "");   // fill background
    mvwprintw(titleWin, 0, 1, "%s", text);
    mvwprintw(titleWin, 0, cols - (int)strlen(ver) - 1, "%s", ver);
    // separator
    mvwhline(titleWin, TITLE_LINES - 1, 0, ACS_HLINE, cols);
    wattroff(titleWin, COLOR_PAIR(1) | A_BOLD);
    wrefresh(titleWin);
}

void FreePLCGui::drawStatus(const std::string& msg) {
    wbkgd(statusWin, COLOR_PAIR(7));
    werase(statusWin);
    int cols = getmaxx(statusWin);
    wattron(statusWin, COLOR_PAIR(7));
    mvwhline(statusWin, 0, 0, ACS_HLINE, cols);
    mvwprintw(statusWin, 1, 1, "%-*s", cols - 2, msg.c_str());
    wattroff(statusWin, COLOR_PAIR(7));
    wrefresh(statusWin);
}

void FreePLCGui::drawIOStatus() {
    auto relay = manager.getCurrentRelay();
    auto prog  = manager.getCurrentProgram();

    werase(ioWin);
    box(ioWin, 0, 0);

    wattron(ioWin, COLOR_PAIR(6) | A_BOLD);
    mvwprintw(ioWin, 1, 2, "I/O Status");
    wattroff(ioWin, COLOR_PAIR(6) | A_BOLD);

    if (!relay) {
        mvwprintw(ioWin, 3, 2, "No relay selected");
        wrefresh(ioWin);
        return;
    }

    // Relay name + run status
    std::string relayLabel = "Relay: " + relay->name;
    mvwprintw(ioWin, 2, 2, "%-*s", IO_COLS - 4, relayLabel.c_str());

    bool running = prog && prog->isRunning();
    if (running) {
        wattron(ioWin, COLOR_PAIR(4) | A_BOLD);
        mvwprintw(ioWin, 3, 2, "[ RUNNING ]");
        wattroff(ioWin, COLOR_PAIR(4) | A_BOLD);
    } else {
        wattron(ioWin, COLOR_PAIR(5));
        mvwprintw(ioWin, 3, 2, "[ STOPPED ]");
        wattroff(ioWin, COLOR_PAIR(5));
    }

    int row = 5;
    wattron(ioWin, A_UNDERLINE);
    mvwprintw(ioWin, row++, 2, "Inputs:");
    wattroff(ioWin, A_UNDERLINE);

    for (const auto& kv : relay->inputs) {
        bool val = kv.second;
        if (val) wattron(ioWin, COLOR_PAIR(4) | A_BOLD);
        else     wattron(ioWin, COLOR_PAIR(5));
        mvwprintw(ioWin, row++, 3, "I%d = %s", kv.first, val ? "ON " : "OFF");
        if (val) wattroff(ioWin, COLOR_PAIR(4) | A_BOLD);
        else     wattroff(ioWin, COLOR_PAIR(5));
        if (row >= getmaxy(ioWin) - 4) break;
    }

    row++;
    wattron(ioWin, A_UNDERLINE);
    mvwprintw(ioWin, row++, 2, "Outputs:");
    wattroff(ioWin, A_UNDERLINE);

    for (const auto& kv : relay->outputs) {
        bool val = kv.second;
        if (val) wattron(ioWin, COLOR_PAIR(4) | A_BOLD);
        else     wattron(ioWin, COLOR_PAIR(5));
        mvwprintw(ioWin, row++, 3, "Q%d = %s", kv.first, val ? "ON " : "OFF");
        if (val) wattroff(ioWin, COLOR_PAIR(4) | A_BOLD);
        else     wattroff(ioWin, COLOR_PAIR(5));
        if (row >= getmaxy(ioWin) - 2) break;
    }

    wrefresh(ioWin);
}

// ============================================================
// Generic menu
// ============================================================

int FreePLCGui::showMenu(const std::string& title,
                         const std::vector<std::string>& items) {
    int selected = 0;
    int n = static_cast<int>(items.size());

    while (true) {
        werase(menuWin);
        box(menuWin, 0, 0);

        int cols = getmaxx(menuWin);

        // Title inside box
        wattron(menuWin, COLOR_PAIR(6) | A_BOLD);
        mvwprintw(menuWin, 1, 2, "%s", title.c_str());
        wattroff(menuWin, COLOR_PAIR(6) | A_BOLD);
        mvwhline(menuWin, 2, 1, ACS_HLINE, cols - 2);

        // Items
        for (int i = 0; i < n; i++) {
            if (i == selected) {
                wattron(menuWin, COLOR_PAIR(3) | A_BOLD);
                mvwprintw(menuWin, 3 + i, 2, " %-*s ", cols - 6, items[i].c_str());
                wattroff(menuWin, COLOR_PAIR(3) | A_BOLD);
            } else {
                wattron(menuWin, COLOR_PAIR(2));
                mvwprintw(menuWin, 3 + i, 2, " %-*s ", cols - 6, items[i].c_str());
                wattroff(menuWin, COLOR_PAIR(2));
            }
        }

        // Footer hint
        mvwprintw(menuWin, getmaxy(menuWin) - 2, 2,
                  "Arrow keys: navigate  Enter: select  q/ESC: back");

        wrefresh(menuWin);
        drawIOStatus();

        int ch = wgetch(menuWin);
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + n) % n;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % n;
                break;
            case '\n':
            case KEY_ENTER:
                return selected;
            case 'q':
            case 27:  // ESC
                return -1;
            default:
                break;
        }
    }
}

// ============================================================
// Input helpers
// ============================================================

std::string FreePLCGui::promptString(const std::string& prompt) {
    werase(menuWin);
    box(menuWin, 0, 0);
    wattron(menuWin, COLOR_PAIR(6));
    mvwprintw(menuWin, 2, 2, "%s", prompt.c_str());
    wattroff(menuWin, COLOR_PAIR(6));
    mvwprintw(menuWin, 3, 2, "> ");
    wrefresh(menuWin);

    echo();
    curs_set(1);
    char buf[64] = {};
    mvwgetnstr(menuWin, 3, 4, buf, 63);
    noecho();
    curs_set(0);
    return std::string(buf);
}

int FreePLCGui::promptInt(const std::string& prompt) {
    std::string s = promptString(prompt);
    try { return std::stoi(s); } catch (...) { return -1; }
}

bool FreePLCGui::promptYesNo(const std::string& prompt) {
    werase(menuWin);
    box(menuWin, 0, 0);
    wattron(menuWin, COLOR_PAIR(6));
    mvwprintw(menuWin, 2, 2, "%s  (y/n)", prompt.c_str());
    wattroff(menuWin, COLOR_PAIR(6));
    wrefresh(menuWin);

    while (true) {
        int ch = wgetch(menuWin);
        if (ch == 'y' || ch == 'Y') return true;
        if (ch == 'n' || ch == 'N' || ch == 27) return false;
    }
}

// ============================================================
// Relay selection screen
// ============================================================

void FreePLCGui::screenRelaySelect() {
    while (true) {
        auto names = manager.getRelayNames();
        std::vector<std::string> items;
        for (const auto& n : names) {
            auto relay = manager.getCurrentRelay();
            std::string marker = (relay && relay->name == n) ? " [active]" : "";
            items.push_back(n + marker);
        }
        items.push_back("+ Create new relay");

        int sel = showMenu("Select Relay", items);
        if (sel < 0) return;

        if (sel == static_cast<int>(names.size())) {
            // Create new relay
            std::string name = promptString("Enter relay name:");
            if (name.empty()) { drawStatus("Cancelled."); continue; }
            if (manager.hasRelay(name)) {
                drawStatus("Relay '" + name + "' already exists.");
                continue;
            }
            int inp = promptInt("Number of inputs (e.g. 6):");
            if (inp <= 0) inp = 6;
            int out = promptInt("Number of outputs (e.g. 6):");
            if (out <= 0) out = 6;
            manager.createRelay(name, inp, out);
            manager.selectRelay(name);
            drawStatus("Relay '" + name + "' created and selected.");
        } else {
            manager.selectRelay(names[sel]);
            drawStatus("Relay '" + names[sel] + "' selected.");
        }
        drawIOStatus();
    }
}

// ============================================================
// LD Programming screen
// ============================================================

void FreePLCGui::screenLDProgram() {
    auto relay = manager.getCurrentRelay();
    auto prog  = manager.getCurrentProgram();

    if (!relay || !prog) {
        drawStatus("No relay selected! Please select a relay first.");
        return;
    }

    while (true) {
        // Build element list for display
        const auto& elems = prog->getElements();
        std::vector<std::string> items;
        items.push_back("--- Add AND gate");
        items.push_back("--- Add OR gate");
        items.push_back("--- Add NOT gate");
        items.push_back("--- Add RS Trigger");
        items.push_back("--- Remove element");
        items.push_back("--- Clear program");
        items.push_back("------- Program listing -------");
        for (size_t i = 0; i < elems.size(); i++) {
            items.push_back("  " + std::to_string(i + 1) + ". " + elems[i]->toString());
        }
        if (elems.empty()) {
            items.push_back("  (program is empty)");
        }

        int sel = showMenu("LD Programming  [relay: " + relay->name + "]", items);
        if (sel < 0) return;

        switch (sel) {
            case 0: {
                int in1 = promptInt("AND gate - Input 1 channel:");
                int in2 = promptInt("AND gate - Input 2 channel:");
                int out  = promptInt("AND gate - Output channel:");
                if (in1 > 0 && in2 > 0 && out > 0) {
                    prog->addElement(std::make_shared<AndGate>(in1, in2, out));
                    drawStatus("AND gate added.");
                } else {
                    drawStatus("Invalid channels - gate not added.");
                }
                break;
            }
            case 1: {
                int in1 = promptInt("OR gate - Input 1 channel:");
                int in2 = promptInt("OR gate - Input 2 channel:");
                int out  = promptInt("OR gate - Output channel:");
                if (in1 > 0 && in2 > 0 && out > 0) {
                    prog->addElement(std::make_shared<OrGate>(in1, in2, out));
                    drawStatus("OR gate added.");
                } else {
                    drawStatus("Invalid channels - gate not added.");
                }
                break;
            }
            case 2: {
                int in  = promptInt("NOT gate - Input channel:");
                int out = promptInt("NOT gate - Output channel:");
                if (in > 0 && out > 0) {
                    prog->addElement(std::make_shared<NotGate>(in, out));
                    drawStatus("NOT gate added.");
                } else {
                    drawStatus("Invalid channels - gate not added.");
                }
                break;
            }
            case 3: {
                int set   = promptInt("RS Trigger - SET input channel:");
                int reset = promptInt("RS Trigger - RESET input channel:");
                int out   = promptInt("RS Trigger - Output channel:");
                if (set > 0 && reset > 0 && out > 0) {
                    prog->addElement(std::make_shared<RSTrigger>(set, reset, out));
                    drawStatus("RS Trigger added.");
                } else {
                    drawStatus("Invalid channels - trigger not added.");
                }
                break;
            }
            case 4: {
                if (prog->getElements().empty()) {
                    drawStatus("Program is empty - nothing to remove.");
                    break;
                }
                int idx = promptInt("Enter element number to remove:");
                prog->removeElement(idx - 1);
                drawStatus("Element removed.");
                break;
            }
            case 5: {
                if (promptYesNo("Clear entire program?")) {
                    prog->clearElements();
                    drawStatus("Program cleared.");
                }
                break;
            }
            default:
                // Clicking on a listing item is informational only
                break;
        }
    }
}

// ============================================================
// Run program screen
// ============================================================

void FreePLCGui::screenRunProgram() {
    auto relay = manager.getCurrentRelay();
    auto prog  = manager.getCurrentProgram();

    if (!relay || !prog) {
        drawStatus("No relay selected! Please select a relay first.");
        return;
    }

    if (prog->getElements().empty()) {
        drawStatus("Program is empty - add logic elements first.");
        return;
    }

    drawStatus("Program RUNNING - Press 'q' or ESC to stop");
    drawIOStatus();

    // Launch program in background thread
    std::thread t([&prog]() { prog->run(); });

    // Refresh I/O display while running
    wtimeout(menuWin, 200);
    while (prog->isRunning()) {
        int ch = wgetch(menuWin);
        drawIOStatus();
        if (ch == 'q' || ch == 27) {
            prog->stop();
        }
    }
    wtimeout(menuWin, -1);  // restore blocking

    if (t.joinable()) t.join();
    drawStatus("Program stopped.");
    drawIOStatus();
}

// ============================================================
// Manual I/O control screen
// ============================================================

void FreePLCGui::screenManualIO() {
    auto relay = manager.getCurrentRelay();
    if (!relay) {
        drawStatus("No relay selected! Please select a relay first.");
        return;
    }

    while (true) {
        std::vector<std::string> items;
        // Dynamic input toggle entries
        for (const auto& kv : relay->inputs) {
            std::string state = kv.second ? "ON " : "OFF";
            items.push_back("Toggle I" + std::to_string(kv.first) + "  [" + state + "]");
        }

        int sel = showMenu("Manual I/O Control  [relay: " + relay->name + "]", items);
        if (sel < 0) return;

        // sel corresponds to input channel index (sorted map)
        int i = 0;
        for (auto& kv : relay->inputs) {
            if (i == sel) {
                bool newVal = !kv.second;
                relay->setInput(kv.first, newVal);
                drawStatus("I" + std::to_string(kv.first) +
                           " set to " + (newVal ? "ON" : "OFF"));

                // If program is running, execute one cycle immediately
                auto prog = manager.getCurrentProgram();
                if (prog && prog->isRunning()) prog->execute();
                break;
            }
            i++;
        }
        drawIOStatus();
    }
}

// ============================================================
// Main run loop
// ============================================================

void FreePLCGui::run() {
    while (true) {
        std::vector<std::string> mainItems = {
            "Select / Create Relay",
            "Program LD (Logic Diagram)",
            "Run Program",
            "Manual I/O Control",
        };

        int sel = showMenu("FreePLC Main Menu", mainItems);
        if (sel < 0) break;  // ESC / q → exit

        switch (sel) {
            case 0: screenRelaySelect(); break;
            case 1: screenLDProgram();   break;
            case 2: screenRunProgram();  break;
            case 3: screenManualIO();    break;
        }
    }

    // Clean goodbye message (shown after endwin() would garble it)
    // endwin() is called in destructor
}
