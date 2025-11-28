#ifndef VORTEX_MICRO_H
#define VORTEX_MICRO_H

#include "vortex_common.h"

class VORTEX_MICRO {
public:
    VORTEX_MICRO();
    void begin(long baud);
    void loop();
    void addCommand(const char* name, CommandCallback callback, const char* help = "");
    void setPrompt(const char* p);

    // Funzione COOPERATIVA per interrompere i comandi in esecuzione
    static bool checkAbort();

private:
    static const int MAX_COMMANDS = 6;
    static const int MAX_INPUT    = 32;
    static const int MAX_ARGS     = 4;

    VortexCommand commands[MAX_COMMANDS];
    int commandCount;
    char inputBuffer[MAX_INPUT];
    int inputPos;
    const char* prompt;

    void handleInput();
    void printHelp();
};

#endif