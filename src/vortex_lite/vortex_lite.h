#ifndef VORTEX_LITE_H
#define VORTEX_LITE_H

#include "vortex_common.h"

class VORTEX_LITE {
public:
    VORTEX_LITE();
    void begin(long baud);
    void loop();
    void addCommand(const char* name, CommandCallback callback, const char* help = "");

    // Funzione COOPERATIVA per interrompere i comandi in esecuzione
    static bool checkAbort();

private:
    static const int MAX_COMMANDS = 10;
    static const int MAX_INPUT    = 64;
    static const int MAX_ARGS     = 6;
    static const int HISTORY_SIZE = 4; // History 4 comandi

    VortexCommand commands[MAX_COMMANDS];
    int commandCount;
    
    char inputBuffer[MAX_INPUT];
    int inputPos;
    
    char history[HISTORY_SIZE][MAX_INPUT];
    int historyHead;
    int historyCount;
    int historyScroll;

    const char* prompt;
    int escapeState;

    void handleInput();
    void saveToHistory(const char* cmd);
    void loadHistory(int direction);
    void printHelp();
    void redrawLine();
};

#endif