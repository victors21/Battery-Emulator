#include "debug_handler.h"
#include <Arduino.h>

void debug_init(void) {
  // Init Serial monitor
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("# Serial initialized");
}

void debug_print(const char* string) {
  if (Serial) {
    Serial.println(string);
  }
}
