#include "vortex_full.h"
#include <string.h>

VORTEX_FULL::VORTEX_FULL() {
    commandCount = 0;
    inputPos = 0;
    prompt = VTX_GREEN "> " VTX_RESET; 
    
    historyHead = 0;
    historyCount = 0;
    historyScroll = -1;
    escapeState = 0;

    memset(inputBuffer, 0, sizeof(inputBuffer));
    memset(history, 0, sizeof(history));

#ifdef VTX_USE_FREERTOS
    taskHandle = NULL;
#endif
}

void VORTEX_FULL::begin(long baud,
                  const char* taskName,
                  uint16_t stackDepth,
                  UBaseType_t priority)
{
    Serial.begin(baud);
    while (!Serial) {} 

    Serial.println();
    Serial.println(VTX_GREEN "=== VORTEX FULL ===" VTX_RESET);
#ifdef VTX_USE_FREERTOS
    Serial.println(VTX_YELLOW "Mode: FreeRTOS (ON)" VTX_RESET);
    if (xTaskCreate(taskWrapper, taskName, stackDepth,
                    this, priority, &taskHandle) == pdPASS) {
        return;
    }
    Serial.println(VTX_RED "Failed to create FreeRTOS task! Falling back to no-FreeRTOS mode." VTX_RESET);
#else
    Serial.println(VTX_YELLOW "Mode: Standalone (OFF)" VTX_RESET);
#endif
    Serial.println("Type 'help' for commands. CTRL+C to interrupt.");
    Serial.print(prompt);
}

void VORTEX_FULL::addCommand(const char* name, CommandCallback callback, const char* help) {
    if (commandCount < MAX_COMMANDS) {
        commands[commandCount++] = {name, callback, help};
    } else {
        Serial.println(VTX_RED "ERR: Max commands reached" VTX_RESET);
    }
}

void VORTEX_FULL::setPrompt(const char* p) {
    prompt = p;
}

// ---------------------------
// FUNZIONI CORE CLI E FULL
// ---------------------------

void VORTEX_FULL::redrawLine() {
    Serial.print("\r");
    Serial.print(prompt);
    Serial.print(inputBuffer);
    Serial.print("\033[K");
}

void VORTEX_FULL::saveToHistory(const char* cmd) {
    int lastIdx = (historyHead - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    if (historyCount > 0 && strcmp(history[lastIdx], cmd) == 0) return;

    strncpy(history[historyHead], cmd, MAX_INPUT - 1);
    history[historyHead][MAX_INPUT - 1] = '\0';

    historyHead = (historyHead + 1) % HISTORY_SIZE;
    if (historyCount < HISTORY_SIZE) historyCount++;
}

void VORTEX_FULL::loadHistory(int direction) {
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

void VORTEX_FULL::handleTabCompletion() {
    if (inputPos == 0) {
        printHelp();
        Serial.print(prompt);
        Serial.print(inputBuffer);
        return;
    }
    
    inputBuffer[inputPos] = 0;
    int matches = 0;
    int lastMatchIndex = -1;

    for (int i = 0; i < commandCount; i++) {
        if (strncmp(commands[i].name, inputBuffer, inputPos) == 0) {
            matches++;
            lastMatchIndex = i;
        }
    }

    if (matches == 1) {
        strcpy(inputBuffer, commands[lastMatchIndex].name);
        inputPos = strlen(inputBuffer);
        redrawLine();
    } else if (matches > 1) {
        Serial.println();
        for (int i = 0; i < commandCount; i++) {
            if (strncmp(commands[i].name, inputBuffer, inputPos) == 0) {
                Serial.print(commands[i].name); Serial.print(" ");
            }
        }
        Serial.println();
        Serial.print(prompt);
        Serial.print(inputBuffer);
    }
}

// Funzione statica cooperativa di interruzione (da chiamare dentro i comandi)
bool VORTEX_FULL::checkAbort() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == KEY_CTRL_C) { 
            return true; 
        }
    }
    return false;
}

void VORTEX_FULL::handleInput(char* input) {
    char* argv[MAX_ARGV];
    int argc = 0;

    // Tokenizzazione
    char* token = strtok(input, " ");
    while (token && argc < MAX_ARGV)
        argv[argc++] = token,
        token = strtok(NULL, " ");

    if (argc == 0) return;

    if (!strcmp(argv[0], "help")) {
        printHelp();
        return;
    }

    for (int i = 0; i < commandCount; i++) {
        if (!strcmp(argv[0], commands[i].name)) {
            commands[i].callback(argc, argv);
            return;
        }
    }

    Serial.print(VTX_RED "Unknown command: " VTX_RESET);
    Serial.println(argv[0]);
}

void VORTEX_FULL::printHelp() {
    Serial.println(VTX_GREEN "Available commands:" VTX_RESET);
    Serial.println(" - help: Show this list");
    for (int i = 0; i < commandCount; i++) {
        Serial.print(" - ");
        Serial.print(commands[i].name);
        if (commands[i].help && strlen(commands[i].help))
            Serial.print(": "), Serial.println(commands[i].help);
        else
            Serial.println();
    }
}

// ---------------------------
// LOOP MODES
// ---------------------------

// Usato solo in modalità Standalone
void VORTEX_FULL::loop() {
#ifndef VTX_USE_FREERTOS
    while (Serial.available()) {
        char c = Serial.read();

        // 1. Sequenze Escape (Frecce)
        if (escapeState == 1) { 
            if (c == '[') escapeState = 2; else escapeState = 0; return;
        }
        if (escapeState == 2) { 
            if (c == 'A') loadHistory(-1); 
            else if (c == 'B') loadHistory(1);  
            escapeState = 0; return;
        }
        if (c == KEY_ESC) { escapeState = 1; return; }

        // 2. Ctrl+C (interruzione input)
        if (c == KEY_CTRL_C) {
            Serial.println("^C");
            inputPos = 0; historyScroll = -1;
            Serial.print(prompt);
            return;
        }

        // 3. TAB COMPLETION
        if (c == KEY_TAB) {
            handleTabCompletion();
            return;
        }
        
        // 4. Invio
        if (c == '\r' || c == '\n') {
            if (inputPos > 0) {
                inputBuffer[inputPos] = 0;
                Serial.println();
                
                // Copia per la history e l'handler (handleInput usa strtok)
                char temp[MAX_INPUT];
                strcpy(temp, inputBuffer);
                saveToHistory(temp);
                handleInput(temp);
                
                inputPos = 0; historyScroll = -1;
            } else Serial.println();
            Serial.print(prompt);
            return;
        }

        // 5. Backspace/Delete
        if (c == KEY_BS || c == KEY_DEL) {
            if (inputPos > 0) {
                inputPos--;
                Serial.print("\b \b");
            }
            return;
        }

        // 6. Char
        if (inputPos < MAX_INPUT - 1 && c >= 32 && c <= 126) {
            inputBuffer[inputPos++] = c;
            Serial.print(c);
        }
    }
#endif
}

#ifdef VTX_USE_FREERTOS
// Wrapper per FreeRTOS
void VORTEX_FULL::taskWrapper(void* pvParameters) {
    reinterpret_cast<VORTEX_FULL*>(pvParameters)->taskLoop();
    vTaskDelete(NULL);
}

// Loop FreeRTOS
void VORTEX_FULL::taskLoop() {
    Serial.println("Type 'help' for commands. CTRL+C to interrupt.");
    Serial.print(prompt);

    while (1) {
        while (Serial.available()) {
            char c = Serial.read();

            // Logica di input (stessa di loop() ma con goto per il break)
            // 1. Sequenze Escape (Frecce)
            if (escapeState == 1) { 
                if (c == '[') escapeState = 2; else escapeState = 0; goto task_yield;
            }
            if (escapeState == 2) { 
                if (c == 'A') loadHistory(-1); 
                else if (c == 'B') loadHistory(1);  
                escapeState = 0; goto task_yield;
            }
            if (c == KEY_ESC) { escapeState = 1; goto task_yield; }

            // 2. Ctrl+C
            if (c == KEY_CTRL_C) {
                Serial.println("^C");
                inputPos = 0; historyScroll = -1;
                Serial.print(prompt);
                goto task_yield;
            }

            // 3. TAB COMPLETION
            if (c == KEY_TAB) {
                handleTabCompletion();
                goto task_yield;
            }
            
            // 4. Invio
            if (c == '\r' || c == '\n') {
                if (inputPos > 0) {
                    inputBuffer[inputPos] = 0;
                    Serial.println();
                    char temp[MAX_INPUT];
                    strcpy(temp, inputBuffer);
                    saveToHistory(temp);
                    handleInput(temp);
                    inputPos = 0; historyScroll = -1;
                } else Serial.println();
                Serial.print(prompt);
                goto task_yield;
            }

            // 5. Backspace/Delete
            if (c == KEY_BS || c == KEY_DEL) {
                if (inputPos > 0) {
                    inputPos--;
                    Serial.print("\b \b");
                }
                goto task_yield;
            }

            // 6. Char
            if (inputPos < MAX_INPUT - 1 && c >= 32 && c <= 126) {
                inputBuffer[inputPos++] = c;
                Serial.print(c);
            }
        }
        
        task_yield:; 
        
        // Yield/Delay per non monopolizzare la CPU
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}
#endif