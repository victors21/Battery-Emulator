#ifndef __HW_LILYGO_H__
#define __HW_LILYGO_H__

#define MAIN_FUNCTION_CORE 1

// PIN mappings, do not change these unless you are adding on extra hardware to the PCB
#define CAN_TX_PIN GPIO_NUM_27
#define CAN_RX_PIN GPIO_NUM_26
#define CAN_SE_PIN 23

#define PIN_5V_EN 16
#define RS485_EN_PIN 17  // 17 /RE
#define RS485_TX_PIN 22  // 21
#define RS485_RX_PIN 21  // 22
#define RS485_SE_PIN 19  // 22 /SHDN

#ifdef DUAL_CAN
#define MCP2515_SCK 12   // SCK input of MCP2515
#define MCP2515_MOSI 5   // SDI input of MCP2515
#define MCP2515_MISO 34  // SDO output of MCP2515 - Pin 34 is input only, without pullup/down resistors
#define MCP2515_CS 18    // CS input of MCP2515
#define MCP2515_INT 35   // INT output of MCP2515 - Pin 35 is input only, without pullup/down resistors
#endif

#ifdef CONTACTOR_CONTROL
#define POSITIVE_CONTACTOR_PIN 32
#define NEGATIVE_CONTACTOR_PIN 33
#define PRECHARGE_PIN 25
#endif

#ifdef BMW_I3_BATTERY
#define WUP_PIN 25
#endif

#define SD_MISO_PIN 2
#define SD_MOSI_PIN 15
#define SD_SCLK_PIN 14
#define SD_CS_PIN 13
#define WS2812_PIN 4

#define LED_MAX_BRIGHTNESS 40

/* ----- Error checks below, don't change ----- */
#ifndef HW_CONFIGURED
#define HW_CONFIGURED
#else
#error Multiple HW defined! Please select a single HW
#endif

#endif
