#include <Arduino.h>

#include "../../system_include.h"
#include "core.h"
#include "led/led.h"

#include "../../system/can/canlayer.h"
#include "../../system/inverter_side/INVERTERS.h"
#include "../../utils/utils.h"

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
}

static LED led(led_mode::LED_MODE_DEFAULT);
static MyTimer rtos_timer(5000);
CanInterface* g_can1_interface;

void core_init(void) {
  led.init();
  g_can1_interface = can1_get_instance(CAN_BAUDRATE_500);
  inverter_side_init();
}

void core_exe(void) {
  led.exe();
  watchdog.kick_battery_side();
  watchdog.kick_inverter_side();

  if (rtos_timer.elapsed()) {
    UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

    Serial.print("High Water Mark: ");
    Serial.println(uxHighWaterMark);
  }
}
