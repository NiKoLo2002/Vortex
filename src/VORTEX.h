#ifndef VORTEX_H
#define VORTEX_H

#include <Arduino.h>

#define VORTEX_VERSION "0.0.3"

// Colors for ANSI escape codes
#define VTX_RESET   "\033[0m"
#define VTX_RED     "\033[1;31m"
#define VTX_GREEN   "\033[1;32m"
#define VTX_YELLOW  "\033[1;33m"
#define VTX_BLUE    "\033[1;34m"
#define VTX_MAGENTA "\033[1;35m"
#define VTX_CYAN    "\033[1;36m"
#define VTX_WHITE   "\033[1;37m"

typedef void (*CommandCallback)(int argc, char** argv);

struct VortexCommand {
    const char* name;
    CommandCallback callback;
    const char* help;
};

class VORTEX {
public:
    VORTEX();
    void begin(long baud);
    void addCommand(const char* name, CommandCallback callback, const char* help="");
    void loop();
    void setPrompt(const char* p);

private:
    static const int MAX_COMMANDS = 20;
    static const int MAX_INPUT = 128;
    VortexCommand commands[MAX_COMMANDS];
    int commandCount;
    char inputBuffer[MAX_INPUT];
    int inputPos;
    const char* prompt;

    void handleInput(char* input);
    void printHelp();
};

#endif
