#ifndef _ESP32CAN_IF_H_
#define _ESP32CAN_IF_H_

#include "../../lib/miwagner-ESP32-Arduino-CAN/ESP32CAN.h"
#include "../../system_include.h"
#include "canlayer.h"

class ESP32CanInterface : public CanInterface {
 private:
  CAN_BAUDRATE baudrate;

 public:
  ESP32CanInterface(CAN_BAUDRATE baudrate) : baudrate(baudrate) {}
  void init(void) override;
  void transmit(const CAN_FRAME_TYPE& frame) override;
};

#endif
