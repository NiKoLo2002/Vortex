#include "VORTEX.h"
#include <string.h>

VORTEX::VORTEX() {
    commandCount = 0;
    inputPos = 0;
    prompt = "\033[1;32m>\033[0m "; // prompt verde
}

void VORTEX::begin(long baud) {
    Serial.begin(baud);
    while (!Serial) {}
    Serial.println(VTX_CYAN "Welcome to VORTEX CLI" VTX_RESET);
    Serial.println("Type 'help' to see commands.");
    Serial.print(prompt);
}

void VORTEX::setPrompt(const char* p) {
    prompt = p;
}

void VORTEX::addCommand(const char* name, CommandCallback callback, const char* help) {
    if (commandCount < MAX_COMMANDS) {
        commands[commandCount].name = name;
        commands[commandCount].callback = callback;
        commands[commandCount].help = help;
        commandCount++;
    }
}

void VORTEX::loop() {
    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\r' || c == '\n') { // gestisce CR, LF o CR+LF
            inputBuffer[inputPos] = '\0';
            if (inputPos > 0) {
                Serial.println();            // va a capo prima dell’output
                handleInput(inputBuffer);    // esegue il comando
            }
            inputPos = 0;
            Serial.print(prompt);           // ristampa il prompt
        } else if (c == 8 || c == 127) {    // backspace
            if (inputPos > 0) {
                inputPos--;
                Serial.print("\b \b");
            }
        } else {
            if (inputPos < MAX_INPUT - 1) {
                inputBuffer[inputPos++] = c;
                Serial.print(c);
            }
        }
    }
}

void VORTEX::handleInput(char* input) {
    char* argv[16];
    int argc = 0;

    char* token = strtok(input, " ");
    while (token != NULL && argc < 16) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) return;

    if (strcmp(argv[0], "help") == 0) {
        printHelp();
        return;
    }

    for (int i = 0; i < commandCount; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].callback(argc, argv);
            return;
        }
    }

    Serial.print(VTX_RED "Unknown command: " VTX_RESET);
    Serial.println(argv[0]);
}

void VORTEX::printHelp() {
    Serial.println(VTX_GREEN "Available commands:" VTX_RESET);
    for (int i = 0; i < commandCount; i++) {
        Serial.print(" - ");
        Serial.print(commands[i].name);
        if (commands[i].help && strlen(commands[i].help) > 0) {
            Serial.print(": ");
            Serial.println(commands[i].help);
        } else {
            Serial.println();
        }
    }
}
