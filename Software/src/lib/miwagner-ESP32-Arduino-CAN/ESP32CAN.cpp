#include "ESP32CAN.h"
#include <Arduino.h>
#include "../../utils/events.h"

CAN_device_t CAN_cfg;

int ESP32CAN::CANInit() {
  return CAN_init();
}
int ESP32CAN::CANWriteFrame(const CAN_frame_t* p_frame) {
  static unsigned long start_time;
  int result = -1;
  if (tx_ok) {
    result = CAN_write_frame(p_frame);
    tx_ok = (result == 0) ? true : false;
    if (tx_ok == false) {
      Serial.println("CAN failure! Check wires");
      set_event(EVENT_CAN_TX_FAILURE, 0);
      start_time = millis();
    } else {
      clear_event(EVENT_CAN_TX_FAILURE);
    }
  } else {
    if ((millis() - start_time) >= 2000) {
      tx_ok = true;
    }
  }
  return result;
}
int ESP32CAN::CANStop() {
  return CAN_stop();
}
int ESP32CAN::CANConfigFilter(const CAN_filter_t* p_filter) {
  return CAN_config_filter(p_filter);
}

void ESP32CAN::CANSetCfg(CAN_device_t* can_cfg) {
  CAN_cfg = *can_cfg;
}

ESP32CAN ESP32_can;
