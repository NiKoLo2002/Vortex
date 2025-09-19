#include <VORTEX.h>

VORTEX cli;

void cmd_status(int argc, char** argv) {
  Serial.println(VTX_YELLOW "System is running in real-time." VTX_RESET);
}

void cmd_echo(int argc, char** argv) {
  Serial.print(VTX_BLUE "Echo:" VTX_RESET);
  for (int i = 1; i < argc; i++) {
    Serial.print(" ");
    Serial.print(argv[i]);
  }
  Serial.println();
}

void setup() {
  cli.begin(115200);
  cli.addCommand("status", cmd_status, "Show system status");
  cli.addCommand("echo", cmd_echo, "Echo back arguments");
}

void loop() {
  cli.loop();
}
