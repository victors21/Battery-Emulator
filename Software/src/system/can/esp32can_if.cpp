#include "esp32can_if.h"
#include "../../system_include.h"

void ESP32CanInterface::init(void) {
  // CAN pins
  pinMode(CAN_SE_PIN, OUTPUT);
  digitalWrite(CAN_SE_PIN, LOW);

  CAN_device_t CAN_cfg;
  CAN_cfg.speed = (CAN_speed_t)baudrate;
  CAN_cfg.tx_pin_id = CAN_TX_PIN;
  CAN_cfg.rx_pin_id = CAN_RX_PIN;
  CAN_cfg.rx_queue = xQueueCreate(CAN_RX_QUEUE_SIZE, sizeof(CAN_frame_t));

  ESP32_can.CANSetCfg(&CAN_cfg);
  // Init CAN Module
  ESP32_can.CANInit();
}

void ESP32CanInterface::transmit(const CAN_FRAME_TYPE& frame) {
  CAN_frame_t esp32can_frame = *(CAN_frame_t*)&frame;
  ESP32_can.CANWriteFrame(&esp32can_frame);
}
