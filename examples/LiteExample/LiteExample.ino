#include "vortex_lite/vortex_lite.h"

// Crea un'istanza della CLI
VORTEX_LITE cli;

// ------------------------------------------------------------------
// COMMANDS
// ------------------------------------------------------------------

// Comando semplice con argomenti
void cmd_echo(int argc, char** argv) {
    if (argc < 2) {
        Serial.println("Usage: echo <text>");
        return;
    }
    Serial.print("ECHO: ");
    // Stampa tutti gli argomenti dopo il comando (argv[0])
    for (int i = 1; i < argc; i++) {
        Serial.print(argv[i]);
        Serial.print(" ");
    }
    Serial.println();
}

// Comando lungo e bloccante (DEVE usare checkAbort)
void cmd_count(int argc, char** argv) {
    long target = 100;
    if (argc > 1) {
        // Conversione dell'argomento a numero
        target = atol(argv[1]);
        if (target <= 0) target = 1;
    }

    Serial.print("Counting up to ");
    Serial.print(target);
    Serial.println("... Press CTRL+C to stop.");

    for (long i = 1; i <= target; i++) {
        // Stampa il numero
        Serial.print(i);
        Serial.print(" ");
        if (i % 20 == 0) Serial.println(); // A capo ogni 20 numeri

        // PICCOLO DELAY: Cruciale in modalità LITE/MICRO per dare tempo
        // a checkAbort() di ricevere i dati seriali!
        delay(10); 

        // CHECK INTERRUPT (Funzione cooperativa)
        if (VORTEX_LITE::checkAbort()) {
            Serial.println("\n--- Interrupted by user! ---");
            return; // Uscita immediata dal comando
        }
    }
    Serial.println("\nDone.");
}

// ------------------------------------------------------------------
// SETUP & LOOP
// ------------------------------------------------------------------

void setup() {
    // Inizializza la CLI a 115200 baud
    cli.begin(115200);

    // Registra i comandi
    cli.addCommand("echo", cmd_echo, "Prints the arguments back.");
    cli.addCommand("count", cmd_count, "Counts up to a number (interruptible). Usage: count [num]");
}

void loop() {
    // In modalità LITE, devi chiamare loop() regolarmente
    cli.loop();

    // Puoi fare altre cose qui, ma non usare delay() lunghi!
}