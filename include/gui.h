// include/gui.h - FreePLCGui: ncurses-based graphical interface
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "relay_manager.h"

// Forward declare ncurses WINDOW to avoid polluting headers
struct _win_st;
typedef struct _win_st WINDOW;

class FreePLCGui {
private:
    RelayManager manager;

    // ncurses windows
    WINDOW* mainWin;
    WINDOW* titleWin;
    WINDOW* menuWin;
    WINDOW* statusWin;
    WINDOW* ioWin;

    int screenRows, screenCols;

    // ---- internal helpers ----
    void initWindows();
    void destroyWindows();
    void drawTitle();
    void drawStatus(const std::string& msg);
    void drawIOStatus();

    // Menu helpers: returns selected index (0-based), -1 = back/exit
    int showMenu(const std::string& title, const std::vector<std::string>& items);
    std::string promptString(const std::string& prompt);
    int promptInt(const std::string& prompt);
    bool promptYesNo(const std::string& prompt);

    // Sub-screens
    void screenRelaySelect();
    void screenLDProgram();
    void screenRunProgram();
    void screenManualIO();

public:
    FreePLCGui();
    ~FreePLCGui();

    void run();
};
