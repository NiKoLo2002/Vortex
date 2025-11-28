#ifndef VORTEX_COMMON_H
#define VORTEX_COMMON_H

#include <Arduino.h>

// Colori ANSI Comuni
#define VTX_RESET   "\033[0m"
#define VTX_RED     "\033[1;31m"
#define VTX_GREEN   "\033[1;32m"
#define VTX_CYAN    "\033[1;36m"
#define VTX_YELLOW  "\033[1;33m"

// Tasti Speciali (ASCII)
#define KEY_CTRL_C  0x03
#define KEY_TAB     0x09
#define KEY_BS      0x08
#define KEY_DEL     0x7F
#define KEY_ESC     0x1B
#define KEY_ENTER   '\r'

// Struttura comando callback
typedef void (*CommandCallback)(int argc, char** argv);

struct VortexCommand {
    const char* name;
    CommandCallback callback;
    const char* help;
};

#endif