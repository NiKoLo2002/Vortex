#include <Arduino.h>
#include "vortex_full/vortex_full.h"

VORTEX_FULL cli;

// ------------------------------------------------------------------
// COMMANDS
// ------------------------------------------------------------------

void cmd_info(int argc, char** argv) {
    Serial.println(VTX_GREEN "System Info:" VTX_RESET);
    Serial.print("  Architettura: ");
    Serial.println(F("ATmega2560 (Standalone)")); // Esempio per Mega
    Serial.print("  Tempo di uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" secondi.");
}

// Comando lungo con checkAbort
void cmd_burn(int argc, char** argv) {
    int duration_s = 5;
    if (argc > 1) duration_s = atoi(argv[1]);
    if (duration_s <= 0) duration_s = 5;

    Serial.print("Avvio operazione di 'stress test' per ");
    Serial.print(duration_s);
    Serial.println(" secondi. (CTRL+C per interrompere)");

    unsigned long startTime = millis();
    unsigned long endTime = startTime + (duration_s * 1000);
    int step = 0;

    while (millis() < endTime) {
        // Operazione simulata (non bloccante a lungo)
        delay(10); 

        if (step % 50 == 0) {
            Serial.print(VTX_YELLOW "." VTX_RESET);
        }

        // CHECK INTERRUPT
        if (VORTEX_FULL::checkAbort()) {
            Serial.println("\n--- Stress test interrotto! ---");
            return;
        }

        step++;
    }

    Serial.println("\nStress test completato.");
}

// ------------------------------------------------------------------
// SETUP & LOOP
// ------------------------------------------------------------------

void setup() {
    // Chiama begin() senza i parametri FreeRTOS. 
    // Verrà stampato "Standalone Mode: ON".
    cli.begin(115200);

    cli.addCommand("info", cmd_info, "Mostra informazioni di sistema.");
    cli.addCommand("burn", cmd_burn, "Simula un'operazione lunga (interrompibile).");
}

void loop() {
    // Chiamata fondamentale per la gestione dell'input seriale
    cli.loop();
    
    // Il resto del codice qui.
}