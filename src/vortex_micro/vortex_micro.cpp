#include "vortex_micro.h"
#include <string.h>

VORTEX_MICRO::VORTEX_MICRO() : commandCount(0), inputPos(0), prompt(VTX_GREEN "> " VTX_RESET) {}

void VORTEX_MICRO::begin(long baud) {
    Serial.begin(baud);
    while (!Serial);
    Serial.println(VTX_GREEN "VORTEX MICRO" VTX_RESET);
    Serial.print(prompt);
}

void VORTEX_MICRO::setPrompt(const char* p) { prompt = p; }

void VORTEX_MICRO::addCommand(const char* name, CommandCallback callback, const char* help) {
    if (commandCount < MAX_COMMANDS) {
        commands[commandCount++] = {name, callback, help};
    }
}

// Funzione statica di interrupt cooperativo (per i comandi lunghi)
bool VORTEX_MICRO::checkAbort() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == KEY_CTRL_C) { 
            return true; 
        }
    }
    return false;
}

void VORTEX_MICRO::loop() {
    while (Serial.available()) {
        char c = Serial.read();

        // 1. Gestione CTRL+C (Interrupt del loop di input)
        if (c == KEY_CTRL_C) {
            Serial.println("^C");
            inputPos = 0;
            Serial.print(prompt);
            return;
        }

        // 2. Invio
        if (c == '\r' || c == '\n') {
            if (inputPos > 0) {
                inputBuffer[inputPos] = 0;
                Serial.println();
                handleInput();
                inputPos = 0;
            } else {
                Serial.println();
            }
            Serial.print(prompt);
            return;
        }

        // 3. Backspace
        if (c == KEY_BS || c == KEY_DEL) {
            if (inputPos > 0) {
                inputPos--;
                Serial.print("\b \b");
            }
            return;
        }

        // 4. Input normale
        if (inputPos < MAX_INPUT - 1 && c >= 32 && c <= 126) {
            inputBuffer[inputPos++] = c;
            Serial.print(c);
        }
    }
}

void VORTEX_MICRO::handleInput() {
    char* argv[MAX_ARGS];
    int argc = 0;
    
    char tempBuf[MAX_INPUT];
    strcpy(tempBuf, inputBuffer); 
    
    char* token = strtok(tempBuf, " ");
    while (token && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return;
    if (strcmp(argv[0], "help") == 0) { printHelp(); return; }

    for (int i = 0; i < commandCount; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].callback(argc, argv);
            return;
        }
    }
    Serial.println(VTX_RED "Unknown" VTX_RESET);
}

void VORTEX_MICRO::printHelp() {
    Serial.println(VTX_GREEN "Available commands:" VTX_RESET);
    Serial.println(" - help: Show this list");
    for (int i = 0; i < commandCount; i++) {
        Serial.print(" - ");
        Serial.print(commands[i].name);
        if (commands[i].help && strlen(commands[i].help) > 0) {
            Serial.print(": ");
            Serial.println(commands[i].help);
        } else Serial.println();
    }
}