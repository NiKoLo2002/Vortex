# VORTEX

**VORTEX** - *Versatile Onboard Real Time EXecutor CLI*

VORTEX is a **modular and robust Arduino library** for creating advanced Command Line Interfaces (CLI). It allows you to easily control your project via serial communication using custom commands, multi-argument support, ANSI colors, and cutting-edge command-line editing features.

---

## 🔹 Acronym
**VORTEX** stands for:
**V**ersatile **O**nboard **R**eal **T**ime **EX**ecutor

---

## 🚀 Key Features

* **Hybrid Multitasking (FULL):** Automatic FreeRTOS detection (including **`arduino_freertos`** for AVR/ATmega) to run the CLI in a **separate RTOS Task**, ensuring maximum system responsiveness.
* **Cooperative Interruption:** All models support the static `checkAbort()` function to allow long-running commands to be interrupted using **CTRL+C**.
* **Advanced Input Management:**
    * **Autocompletion (FULL):** **TAB** functionality to quickly complete command names.
    * **History & Arrows (FULL/LITE):** Navigation through command **history** using **UP/DOWN** arrow keys.
* **Simple, Powerful CLI Core:**
    * Supports commands with **multiple arguments** (`argc`, `argv`).
    * Fully customizable user commands.
    * Colored output and configurable prompt using **ANSI colors**.
* **Standard Terminal Compatibility:** Full compatibility with ANSI terminals like **PuTTY** or **Termux**.
* **Robustness:** Reliable handling of line terminators (`CR`, `LF`, or `CR+LF`).

---

## 🧩 VORTEX Models: Choose Your Power Level

VORTEX is divided into three models to perfectly fit the available RAM on your microcontroller.

| Model | Ideal Platform | Key Features | Memory Footprint (RAM/Flash) |
| :--- | :--- | :--- | :--- |
| **🥇 VORTEX FULL** | ESP32, STM32, ATmega (with FreeRTOS) | **RTOS Support**, **TAB** Completion, **History (8 commands)**, Arrows 🔼/🔽, CTRL+C. | High |
| **🥈 VORTEX LITE** | **ATmega328P (Uno/Nano)**, ATmega2560 (Standalone) | **Optimized History** (4-8 commands), Arrows 🔼/🔽, CTRL+C. | Medium-Low |
| **🥉 VORTEX MICRO** | ATtiny, Ultra-Limited Projects | Command and argument parsing only, minimum overhead. | Minimum |

---

## 💻 Installation

1.  Download the required `.h` and `.cpp` files for your chosen model (`vortex_full`, `vortex_lite`, or `vortex_micro`).
2.  Include the `vortex_common.h` file and the header/source pair of your model (e.g., `vortex_lite.h` and `vortex_lite.cpp`) in your project folder.
3.  For **FULL** mode with FreeRTOS, ensure you have the `arduino_freertos` library (for AVR) or that your platform (like ESP32) has integrated FreeRTOS support.

---

## 💡 Usage Example (VORTEX FULL with FreeRTOS)

```cpp
#include <Arduino_FreeRTOS.h> // For ATmega/AVR
#include "vortex_full.h" 

VORTEX_FULL cli;

void cmd_long_op(int argc, char** argv) {
    Serial.println("Starting long operation... Press CTRL+C to interrupt.");
    
    for (int i = 0; i < 50; i++) {
        // In an RTOS task, use vTaskDelay instead of delay()
        vTaskDelay(pdMS_TO_TICKS(100)); 

        // CRUCIAL: Always check if the user has pressed CTRL+C
        if (VORTEX_FULL::checkAbort()) {
            Serial.println("\nOperation interrupted by user.");
            return;
        }
    }
    Serial.println("\nOperation complete.");
}

void setup() {
    // The CLI will start as a separate RTOS Task.
    // Stack: 400 for AVR/ATmega is a good starting value.
    cli.begin(115200, "VortexCLI", 400, 1);
    
    cli.addCommand("test", cmd_long_op, "Runs an interruptible operation.");
}

void loop() {
    // Leave loop() empty when using FreeRTOS.
}
