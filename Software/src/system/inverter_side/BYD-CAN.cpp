#include "INVERTERS.h"
#ifdef BYD_CAN
#include "../../utils/utils.h"
#include "../datalayer/datalayer.h"
#include "BYD-CAN.h"

static MyTimer timer_2s(2000);
static MyTimer timer_10s(10000);
static MyTimer timer_60s(60000);
static MyTimer timer_update(5000);

/* Do not change code below unless you are sure what you are doing */
// static unsigned long previousMillis2s = 0;   // will store last time a 2s CAN Message was send
// static unsigned long previousMillis10s = 0;  // will store last time a 10s CAN Message was send
// static unsigned long previousMillis60s = 0;  // will store last time a 60s CAN Message was send
// static const int interval2s = 2000;          // interval (ms) at which send CAN Messages
// static const int interval10s = 10000;        // interval (ms) at which send CAN Messages
// static const int interval60s = 60000;        // interval (ms) at which send CAN Messages
static uint8_t char1_151 = 0;
static uint8_t char2_151 = 0;
static uint8_t char3_151 = 0;
static uint8_t char4_151 = 0;
static uint8_t char5_151 = 0;
static uint8_t char6_151 = 0;
static uint8_t char7_151 = 0;

//Startup messages
const CAN_FRAME_TYPE BYD_250 = {
    .FIR = {.B =
                {
                    .DLC = 8,
                    .FF = CAN_STANDARD_FRAME,
                }},
    .MsgID = 0x250,
    .data = {0x03, 0x16, 0x00, 0x66, (uint8_t)((BATTERY_WH_MAX / 100) >> 8), (uint8_t)(BATTERY_WH_MAX / 100), 0x02,
             0x09}};  //3.16 FW , Capacity kWh byte4&5 (example 24kWh = 240)
const CAN_FRAME_TYPE BYD_290 = {.FIR = {.B =
                                            {
                                                .DLC = 8,
                                                .FF = CAN_STANDARD_FRAME,
                                            }},
                                .MsgID = 0x290,
                                .data = {0x06, 0x37, 0x10, 0xD9, 0x00, 0x00, 0x00, 0x00}};
const CAN_FRAME_TYPE BYD_2D0 = {.FIR = {.B =
                                            {
                                                .DLC = 8,
                                                .FF = CAN_STANDARD_FRAME,
                                            }},
                                .MsgID = 0x2D0,
                                .data = {0x00, 0x42, 0x59, 0x44, 0x00, 0x00, 0x00, 0x00}};  //BYD
const CAN_FRAME_TYPE BYD_3D0_0 = {.FIR = {.B =
                                              {
                                                  .DLC = 8,
                                                  .FF = CAN_STANDARD_FRAME,
                                              }},
                                  .MsgID = 0x3D0,
                                  .data = {0x00, 0x42, 0x61, 0x74, 0x74, 0x65, 0x72, 0x79}};  //Battery
const CAN_FRAME_TYPE BYD_3D0_1 = {.FIR = {.B =
                                              {
                                                  .DLC = 8,
                                                  .FF = CAN_STANDARD_FRAME,
                                              }},
                                  .MsgID = 0x3D0,
                                  .data = {0x01, 0x2D, 0x42, 0x6F, 0x78, 0x20, 0x50, 0x72}};  //-Box Pr
const CAN_FRAME_TYPE BYD_3D0_2 = {.FIR = {.B =
                                              {
                                                  .DLC = 8,
                                                  .FF = CAN_STANDARD_FRAME,
                                              }},
                                  .MsgID = 0x3D0,
                                  .data = {0x02, 0x65, 0x6D, 0x69, 0x75, 0x6D, 0x20, 0x48}};  //emium H
const CAN_FRAME_TYPE BYD_3D0_3 = {.FIR = {.B =
                                              {
                                                  .DLC = 8,
                                                  .FF = CAN_STANDARD_FRAME,
                                              }},
                                  .MsgID = 0x3D0,
                                  .data = {0x03, 0x56, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00}};  //VS
//Actual content messages
CAN_FRAME_TYPE BYD_110 = {.FIR = {.B =
                                      {
                                          .DLC = 8,
                                          .FF = CAN_STANDARD_FRAME,
                                      }},
                          .MsgID = 0x110,
                          .data = {0x01, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
CAN_FRAME_TYPE BYD_150 = {.FIR = {.B =
                                      {
                                          .DLC = 8,
                                          .FF = CAN_STANDARD_FRAME,
                                      }},
                          .MsgID = 0x150,
                          .data = {0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00}};
CAN_FRAME_TYPE BYD_190 = {.FIR = {.B =
                                      {
                                          .DLC = 8,
                                          .FF = CAN_STANDARD_FRAME,
                                      }},
                          .MsgID = 0x190,
                          .data = {0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}};
CAN_FRAME_TYPE BYD_1D0 = {.FIR = {.B =
                                      {
                                          .DLC = 8,
                                          .FF = CAN_STANDARD_FRAME,
                                      }},
                          .MsgID = 0x1D0,
                          .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x08}};
CAN_FRAME_TYPE BYD_210 = {.FIR = {.B =
                                      {
                                          .DLC = 8,
                                          .FF = CAN_STANDARD_FRAME,
                                      }},
                          .MsgID = 0x210,
                          .data = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static uint16_t inverter_voltage = 0;
static uint16_t inverter_SOC = 0;
static long inverter_timestamp = 0;
static bool initialDataSent = false;

static void update_values_can_byd();
static void send_can_byd();
static void receive_can_byd(CAN_FRAME_TYPE rx_frame);
static void send_initial_data();

void inverter_side_init(void) {}
void inverter_side_exe(void) {
  if (timer_update.elapsed()) {
    update_values_can_byd();
  }
  send_can_byd();
}

static void
update_values_can_byd() {  //This function maps all the values fetched from battery CAN to the correct CAN messages
  // Initialize variables with scale
  uint16_t max_design_voltage_dV = datalayer.battery.data.max_design_voltage_V * 10;
  uint16_t min_design_voltage_dV = datalayer.battery.data.min_design_voltage_V * 10;
  uint16_t scaled_SOC_pptt = datalayer.battery.status.soc_pct.get_soc() * 10;
  uint16_t SOH_pptt = datalayer.battery.status.soh_pct * 100;
  int16_t temp_min_dC = datalayer.battery.status.temperature_min_C * 10;
  int16_t temp_max_dC = datalayer.battery.status.temperature_max_C * 10;
  uint16_t battery_voltage_dV = datalayer.battery.status.voltage_V * 10;
  int16_t battery_current_dA = datalayer.battery.status.current_A * 10;

  //Calculate values
  uint16_t charge_current_dA =
      ((datalayer.battery.status.max_charge_power_W) / datalayer.battery.data.max_design_voltage_V) *
      10;  //Charge current in dA
  CONSTRAIN(charge_current_dA, 0, MAXCHARGEAMP);

  uint16_t discharge_current_dA =
      ((datalayer.battery.status.max_discharge_power_W) / datalayer.battery.data.max_design_voltage_V) *
      10;  //Discharge current in dA
  CONSTRAIN(discharge_current_dA, 0, MAXDISCHARGEAMP);

  int16_t temperature_average = (temp_min_dC + temp_max_dC) / 2;

  //Map values to CAN messages

  //Maxvoltage (eg 400.0V = 4000 , 16bits long)
  BYD_110.data.u8[0] = (max_design_voltage_dV >> 8);
  BYD_110.data.u8[1] = (max_design_voltage_dV & 0x00FF);
  //Minvoltage (eg 300.0V = 3000 , 16bits long)
  BYD_110.data.u8[2] = (min_design_voltage_dV >> 8);
  BYD_110.data.u8[3] = (min_design_voltage_dV & 0x00FF);
  //Maximum discharge power allowed (Unit: A+1)
  BYD_110.data.u8[4] = (discharge_current_dA >> 8);
  BYD_110.data.u8[5] = (discharge_current_dA & 0x00FF);
  //Maximum charge power allowed (Unit: A+1)
  BYD_110.data.u8[6] = (charge_current_dA >> 8);
  BYD_110.data.u8[7] = (charge_current_dA & 0x00FF);

  //SOC (100.00%)
  BYD_150.data.u8[0] = (scaled_SOC_pptt >> 8);
  BYD_150.data.u8[1] = (scaled_SOC_pptt & 0x00FF);
  //StateOfHealth (100.00%)
  BYD_150.data.u8[2] = (SOH_pptt >> 8);
  BYD_150.data.u8[3] = (SOH_pptt & 0x00FF);
  //Maximum discharge power allowed (Unit: A+1)
  BYD_150.data.u8[4] = (discharge_current_dA >> 8);
  BYD_150.data.u8[5] = (discharge_current_dA & 0x00FF);
  //Maximum charge power allowed (Unit: A+1)
  BYD_150.data.u8[6] = (charge_current_dA >> 8);
  BYD_150.data.u8[7] = (charge_current_dA & 0x00FF);

  //Voltage (ex 370.0)
  BYD_1D0.data.u8[0] = (battery_voltage_dV >> 8);
  BYD_1D0.data.u8[1] = (battery_voltage_dV & 0x00FF);
  //Current (ex 81.0A)
  BYD_1D0.data.u8[2] = (battery_current_dA >> 8);
  BYD_1D0.data.u8[3] = (battery_current_dA & 0x00FF);
  //Temperature average
  BYD_1D0.data.u8[4] = (temperature_average >> 8);
  BYD_1D0.data.u8[5] = (temperature_average & 0x00FF);

  //Temperature max
  BYD_210.data.u8[0] = (temp_max_dC >> 8);
  BYD_210.data.u8[1] = (temp_max_dC & 0x00FF);
  //Temperature min
  BYD_210.data.u8[2] = (temp_min_dC >> 8);
  BYD_210.data.u8[3] = (temp_min_dC & 0x00FF);

#ifdef DEBUG_VIA_USB
  if (char1_151 != 0) {
    Serial.print("Detected inverter: ");
    Serial.print((char)char1_151);
    Serial.print((char)char2_151);
    Serial.print((char)char3_151);
    Serial.print((char)char4_151);
    Serial.print((char)char5_151);
    Serial.print((char)char6_151);
    Serial.println((char)char7_151);
  }
#endif
}

static void receive_can_byd(CAN_FRAME_TYPE rx_frame) {
  switch (rx_frame.MsgID) {
    case 0x151:  //Message originating from BYD HVS compatible inverter. Reply with CAN identifier!
      if (rx_frame.data.u8[0] & 0x01) {  //Battery requests identification
        send_initial_data();
      } else {  // We can identify what inverter type we are connected to
        char1_151 = rx_frame.data.u8[1];
        char2_151 = rx_frame.data.u8[2];
        char3_151 = rx_frame.data.u8[3];
        char4_151 = rx_frame.data.u8[4];
        char5_151 = rx_frame.data.u8[5];
        char6_151 = rx_frame.data.u8[6];
        char7_151 = rx_frame.data.u8[7];
      }
      break;
    case 0x091:
      inverter_voltage = ((rx_frame.data.u8[1] << 8) | rx_frame.data.u8[0]) * 0.1;
      break;
    case 0x0D1:
      inverter_SOC = ((rx_frame.data.u8[1] << 8) | rx_frame.data.u8[0]) * 0.1;
      break;
    case 0x111:
      inverter_timestamp = ((rx_frame.data.u8[3] << 24) | (rx_frame.data.u8[2] << 16) | (rx_frame.data.u8[1] << 8) |
                            rx_frame.data.u8[0]);
      break;
    default:
      break;
  }
}

static void send_can_byd() {
  // Send initial CAN data once on bootup
  if (initialDataSent == false) {
    send_initial_data();
    initialDataSent = true;
  }

  // Send 2s CAN Message
  if (timer_2s.elapsed()) {
    g_can1_interface->transmit(BYD_110);
  }
  // Send 10s CAN Message
  if (timer_10s.elapsed()) {
    g_can1_interface->transmit(BYD_150);
    g_can1_interface->transmit(BYD_1D0);
    g_can1_interface->transmit(BYD_210);
  }
  //Send 60s message
  if (timer_60s.elapsed()) {
    g_can1_interface->transmit(BYD_190);
  }
}

static void send_initial_data() {
  g_can1_interface->transmit(BYD_250);
  g_can1_interface->transmit(BYD_290);
  g_can1_interface->transmit(BYD_2D0);
  g_can1_interface->transmit(BYD_3D0_0);
  g_can1_interface->transmit(BYD_3D0_1);
  g_can1_interface->transmit(BYD_3D0_2);
  g_can1_interface->transmit(BYD_3D0_3);
}
#endif
