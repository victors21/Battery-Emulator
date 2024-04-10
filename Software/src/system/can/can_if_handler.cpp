#include <memory>
#include "canlayer.h"
#include "esp32can_if.h"

static CanInterface* can_interface;

CanInterface* can1_get_instance(CAN_BAUDRATE baudrate) {
#if CAN_1 == ESP32CAN
  return new ESP32CanInterface(baudrate);
#else
  // Nothing yet
  return nullptr;
#endif
}
