#include "vortex_lite.h"
#include <string.h>

VORTEX_LITE::VORTEX_LITE() {
    commandCount = 0; inputPos = 0; escapeState = 0;
    historyHead = 0; historyCount = 0; historyScroll = -1;
    prompt =   VTX_GREEN "> " VTX_RESET;
}

void VORTEX_LITE::begin(long baud) {
    Serial.begin(baud);
    while(!Serial);
    Serial.println(VTX_GREEN "VORTEX LITE" VTX_RESET);
    Serial.print(prompt);
}

void VORTEX_LITE::addCommand(const char* name, CommandCallback callback, const char* help) {
    if(commandCount < MAX_COMMANDS) commands[commandCount++] = {name, callback, help};
}

// Funzione statica di interrupt cooperativo (per i comandi lunghi)
bool VORTEX_LITE::checkAbort() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == KEY_CTRL_C) { 
            return true; 
        }
    }
    return false;
}

void VORTEX_LITE::redrawLine() {
    Serial.print("\r");
    Serial.print(prompt);
    Serial.print(inputBuffer);
    Serial.print("\033[K");
}

void VORTEX_LITE::loadHistory(int direction) {
    if (historyCount == 0) return;

    if (historyScroll == -1) {
        if (direction == -1) historyScroll = (historyHead - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    } else {
        if (direction == -1) { 
             historyScroll = (historyScroll - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        } else {
             if (historyScroll == (historyHead - 1 + HISTORY_SIZE) % HISTORY_SIZE) {
                 historyScroll = -1;
                 inputPos = 0;
                 redrawLine();
                 return;
             }
             historyScroll = (historyScroll + 1) % HISTORY_SIZE;
        }
    }

    if (historyScroll != -1) {
        strcpy(inputBuffer, history[historyScroll]);
        inputPos = strlen(inputBuffer);
        redrawLine();
    }
}

void VORTEX_LITE::loop() {
    while (Serial.available()) {
        char c = Serial.read();

        // --- Gestione Sequenze Escape (Frecce) ---
        if (escapeState == 1) { 
            if (c == '[') escapeState = 2; else escapeState = 0;
            return;
        }
        if (escapeState == 2) { 
            if (c == 'A') loadHistory(-1); 
            else if (c == 'B') loadHistory(1);  
            escapeState = 0;
            return;
        }
        if (c == KEY_ESC) { escapeState = 1; return; }
        // ----------------------------------------

        if (c == KEY_CTRL_C) {
            Serial.println("^C");
            inputPos = 0;
            historyScroll = -1; 
            Serial.print(prompt);
            return;
        }

        if (c == '\r' || c == '\n') {
            if (inputPos > 0) {
                inputBuffer[inputPos] = 0;
                Serial.println();
                
                saveToHistory(inputBuffer); 
                handleInput();
                
                inputPos = 0;
                historyScroll = -1;
            } else {
                Serial.println();
            }
            Serial.print(prompt);
            return;
        }

        if (c == KEY_BS || c == KEY_DEL) {
            if (inputPos > 0) {
                inputPos--;
                Serial.print("\b \b");
            }
            return;
        }

        if (inputPos < MAX_INPUT - 1 && c >= 32 && c <= 126) {
            inputBuffer[inputPos++] = c;
            Serial.print(c);
        }
    }
}

void VORTEX_LITE::handleInput() {
    char tempBuf[MAX_INPUT];
    strcpy(tempBuf, inputBuffer);

    char* argv[MAX_ARGS];
    int argc = 0;
    char* token = strtok(tempBuf, " ");
    while(token && argc < MAX_ARGS) { argv[argc++] = token; token = strtok(NULL, " "); }

    if(argc == 0) return;
    if(strcmp(argv[0], "help") == 0) { printHelp(); return; }

    for(int i=0; i<commandCount; i++){
        if(strcmp(argv[0], commands[i].name) == 0){
            commands[i].callback(argc, argv);
            return;
        }
    }
    Serial.println(VTX_RED "Unknown command." VTX_RESET);
}

void VORTEX_LITE::saveToHistory(const char* cmd) {
    int lastIdx = (historyHead - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    if (historyCount > 0 && strcmp(history[lastIdx], cmd) == 0) return;

    strcpy(history[historyHead], cmd);
    historyHead = (historyHead + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) historyCount++;
}

void VORTEX_LITE::printHelp() {
    Serial.println(VTX_YELLOW "Available commands:" VTX_RESET);
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