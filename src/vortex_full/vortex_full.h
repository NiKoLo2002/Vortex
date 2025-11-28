#ifndef VORTEX_FULL_H
#define VORTEX_FULL_H

#include "vortex_common.h"

// Rilevamento automatico FreeRTOS
#if __has_include("Arduino_FreeRTOS.h") || __has_include("FreeRTOS.h") || defined(ARDUINO_ARCH_ESP32)
  #define VTX_USE_FREERTOS
  
  #if __has_include("Arduino_FreeRTOS.h")
    #include <Arduino_FreeRTOS.h>
  #elif __has_include("FreeRTOS.h")
    #include <FreeRTOS.h>
  #endif
  
  #include <task.h> // Common task header
#endif

class VORTEX_FULL {
public:
    VORTEX_FULL();

    // begin() tenta di avviare il Task RTOS se VTX_USE_FREERTOS è definito
    void begin(long baud,
               const char* taskName = "VortexCLI",
               uint16_t stackDepth = 2048, // Stack più grande per FreeRTOS
               UBaseType_t priority = 1);

    // Deve essere chiamato solo in modalità non-FreeRTOS (Standalone)
    void loop();

    void addCommand(const char* name, CommandCallback callback, const char* help="");
    void setPrompt(const char* p);

    // Funzione COOPERATIVA per l'interruzione dei comandi lunghi
    static bool checkAbort();

private:
    static const int MAX_COMMANDS = 20;
    static const int MAX_INPUT    = 128; // Lunghezza massima input
    static const int MAX_ARGV     = 16;  // Max argomenti
    static const int HISTORY_SIZE = 8;   // History di 8 comandi

    VortexCommand commands[MAX_COMMANDS];
    int commandCount;

    char inputBuffer[MAX_INPUT];
    int inputPos;
    const char* prompt;
    int escapeState; // Per il parsing delle sequenze ESC (frecce)

    // History
    char history[HISTORY_SIZE][MAX_INPUT];
    int historyHead;
    int historyCount;
    int historyScroll;

#ifdef VTX_USE_FREERTOS
    TaskHandle_t taskHandle;
    static void taskWrapper(void* pvParameters);
    void taskLoop();
#endif

    // Funzioni Core CLI e FULL
    void handleInput(char* input);
    void printHelp();
    void saveToHistory(const char* cmd);
    void loadHistory(int direction);
    void handleTabCompletion();
    void redrawLine();
};

#endif