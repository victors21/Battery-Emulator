/* Do not change any code below this line unless you are sure what you are doing */
/* Only change battery specific settings in "USER_SETTINGS.h" */

#include <Arduino.h>
#include <EEPROM.h>
#include "HardwareSerial.h"

#include "src/system_include.h"
#include "src/tasks/core/core.h"

// The current software version
const char* version_number = "6.0.0";

WatchdogHandler watchdog;

// Initialization
void setup() {
  EEPROM.begin(1024);

  debug_init();

  watchdog_init();

  task_init();

  // BOOT button at runtime is used as an input for various things
  pinMode(0, INPUT_PULLUP);
}

// Perform main program functions
void loop() {
  // Attempt to reset the watchdog timer
  watchdog.reset();
}

void watchdog_init(void) {
  watchdog.check_reset_reason();
  esp_task_wdt_init(TASK_BACKGROUND_WATCHDOG_PERIOD, true);  // Timeout period and panic handle true
  esp_task_wdt_add(NULL);                                    // Add current thread to watchdog
}

void task_init(void) {
  // Core
  xTaskCreatePinnedToCore(core_task,          // Task function
                          "core_task",        // Name of the task
                          1024,               // Stack size (in words)
                          NULL,               // Task input parameter
                          TASK_CORE_PRIO,     // Priority of the task
                          NULL,               // Task handle
                          MAIN_FUNCTION_CORE  // Core number
  );
}
