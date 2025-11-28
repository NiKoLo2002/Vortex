#include <Arduino.h>
#include <vortex_micro/vortex_micro.h>

VORTEX_MICRO cli;

// Esempio di funzione callback
void cmd_led(int argc, char** argv) {
    if (argc < 2) {
        Serial.println("Usage: led [on|off]");
        return;
    }
    
    if (strcmp(argv[1], "on") == 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("LED ON");
    } else if (strcmp(argv[1], "off") == 0) {
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("LED OFF");
    } else {
        Serial.println("Invalid arg");
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    cli.begin(115200);
    cli.addCommand("led", cmd_led, "Control LED (usage: led on/off)");
}

void loop() {
    cli.loop();
    // Qui puoi fare altro codice non bloccante
}