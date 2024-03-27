#include "../../system_include.h"

#include <Arduino.h>
#include "../../lib/miwagner-ESP32-Arduino-CAN/CAN_config.h"
#include "../../lib/miwagner-ESP32-Arduino-CAN/ESP32CAN.h"
#include "core.h"
#include "datalayer/datalayer.h"
#include "led.h"

LED led(led_mode::LED_MODE_DEFAULT);

CAN_device_t CAN_cfg;  // CAN Config

static void can_init(void);

void core_task(void* parameter) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(10);  // Convert 10ms to ticks

  // Initialize the xLastWakeTime variable with the current time.

  can_init();
  led.init();

  Serial.println(map_float(0.000f, 0.0f, 0.1f, 0.2f, 0.3f));
  Serial.println(map_float(0.025f, 0.0f, 0.1f, 0.2f, 0.3f));
  Serial.println(map_float(0.050f, 0.0f, 0.1f, 0.2f, 0.3f));
  Serial.println(map_float(0.075f, 0.0f, 0.1f, 0.2f, 0.3f));
  Serial.println(map_float(0.100f, 0.0f, 0.1f, 0.2f, 0.3f));

  Serial.println(map_float(0.000f, 0.0f, 0.1f, 0.3f, 0.2f));
  Serial.println(map_float(0.025f, 0.0f, 0.1f, 0.3f, 0.2f));
  Serial.println(map_float(0.050f, 0.0f, 0.1f, 0.3f, 0.2f));
  Serial.println(map_float(0.075f, 0.0f, 0.1f, 0.3f, 0.2f));
  Serial.println(map_float(0.100f, 0.0f, 0.1f, 0.3f, 0.2f));

  for (;;) {
    led.run();
    watchdog.kick_battery_side();
    watchdog.kick_inverter_side();
    delay(1);
  }
}

static void can_init() {
  // CAN pins
  pinMode(CAN_SE_PIN, OUTPUT);
  digitalWrite(CAN_SE_PIN, LOW);
  CAN_cfg.speed = CAN_BAUD_RATE;
  CAN_cfg.tx_pin_id = CAN_TX_PIN;
  CAN_cfg.rx_pin_id = CAN_RX_PIN;
  CAN_cfg.rx_queue = xQueueCreate(CAN_RX_QUEUE_SIZE, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();

#ifdef DUAL_CAN
  Serial.println("Dual CAN Bus (ESP32+MCP2515) selected");
  gBuffer.initWithSize(25);
  SPI.begin(MCP2515_SCK, MCP2515_MISO, MCP2515_MOSI);
  Serial.println("Configure ACAN2515");
  ACAN2515Settings settings(QUARTZ_FREQUENCY, 500UL * 1000UL);  // CAN bit rate 500 kb/s
  settings.mRequestedMode = ACAN2515Settings::NormalMode;       // Select loopback mode
  can.begin(settings, [] { can.isr(); });
#endif
}
