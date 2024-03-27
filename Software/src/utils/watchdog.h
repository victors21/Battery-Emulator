#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#include <Arduino.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

class WatchdogHandler {

 public:
  void kick_battery_side(void) { wdt_bat_side = true; }
  void kick_inverter_side(void) { wdt_inv_side = true; }
  void reset(void) {
    if (is_complete()) {
      wdt_bat_side = false;
      wdt_inv_side = false;
      esp_task_wdt_reset();
    }
  }
  void check_reset_reason(void) {
    esp_reset_reason_t reason = esp_reset_reason();

    if (reason == ESP_RST_TASK_WDT) {
      // Increment and store the reset counter in case of a watchdog reset?
    }
  }

 private:
  bool wdt_bat_side = false;
  bool wdt_inv_side = false;
  bool is_complete(void) { return wdt_bat_side && wdt_inv_side; }
};

extern WatchdogHandler watchdog;

#endif
