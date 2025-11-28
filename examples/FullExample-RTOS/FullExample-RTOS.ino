#include <vortex_full/vortex_full.h>
#include <arduino_FreeRTOS.h>
// DEVI includere la libreria specifica FreeRTOS nel tuo sketch
// Per ESP32, è automatico. Per AVR, dovrai includere <Arduino_FreeRTOS.h> qui.

VORTEX_FULL cli;

// ------------------------------------------------------------------
// COMMANDS
// ------------------------------------------------------------------

// Comando che dimostra la gestione dell'argomento numerico
void cmd_blink(int argc, char** argv) {
    int pin = 2; // Pin del LED on-board di ESP32
    int times = 3;

    if (argc > 1) {
        times = atoi(argv[1]);
    }
    
    Serial.print("Blinking LED on PIN ");
    Serial.print(pin);
    Serial.print(" ");
    Serial.print(times);
    Serial.println(" times (CTRL+C to interrupt)");

    pinMode(pin, OUTPUT);

    for (int i = 0; i < times; i++) {
        digitalWrite(pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(200)); // Usiamo vTaskDelay invece di delay!
        
        // CHECK INTERRUPT
        if (VORTEX_FULL::checkAbort()) {
            Serial.println("\n--- Blink Aborted! ---");
            digitalWrite(pin, LOW); // Spegni il LED prima di uscire
            return;
        }

        digitalWrite(pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(200));
        
        // CHECK INTERRUPT
        if (VORTEX_FULL::checkAbort()) {
            Serial.println("\n--- Blink Aborted! ---");
            digitalWrite(pin, LOW);
            return;
        }
    }
    Serial.println("\nBlink done.");
}

// Un task di background indipendente dal CLI
void backgroundTask(void* pvParameters) {
    (void) pvParameters; // Ignora il parametro (necessario per RTOS)
    for(;;) {
        // Questo task può fare altre cose mentre la CLI è in attesa di input
        // Serial.println("Background tick..."); // Potrebbe intasare la seriale
        vTaskDelay(pdMS_TO_TICKS(5000)); 
    }
}

// ------------------------------------------------------------------
// SETUP 
// ------------------------------------------------------------------

void setup() {
    // Avvia la CLI. Passando i parametri, begin() creerà il Task RTOS.
    // Stack di 3000 è sicuro per ESP32. Priorità 1 è ok.
    cli.begin(115200, "VortexCLI", 3000, 1);

    cli.addCommand("blink", cmd_blink, "Blinks the LED x times (interruptible). Usage: blink [times]");
    cli.addCommand("status", [](int argc, char** argv){ Serial.println("System status: OK"); }, "Checks system status.");
    
    // Crea un task di background. La CLI girerà in un task separato.
    xTaskCreate(backgroundTask, "BkgTask", 2048, NULL, 0, NULL);
}

// Il loop() deve essere vuoto in FreeRTOS, poiché i task girano in background.
void loop() {
    // NOTA: Se VTX_USE_FREERTOS è definito, cli.loop() non dovrebbe essere chiamato qui.
    // L'inizializzazione in begin() crea un Task che chiama taskLoop() al posto di loop().
}