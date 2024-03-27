#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <Arduino.h>
#include "../system_include.h"

class Settings {
 public:
  Settings() {}

 private:
  /*
  //extern volatile uint32_t BATTERY_WH_MAX;
  //extern volatile uint16_t MAXPERCENTAGE;
  //extern volatile uint16_t MINPERCENTAGE;
  extern volatile uint16_t MAXCHARGEAMP;
  extern volatile uint16_t MAXDISCHARGEAMP;
  extern volatile uint8_t AccessPointEnabled;
  extern volatile bool USE_SCALED_SOC;*/
  bool load_at_boot;
  bool wifi_ap_enabled;
  bool enable_scaled_soc;
  uint32_t battery_Wh_max;
  float min_real_soc;
  float max_real_soc;
  uint16_t battery_min_discharge_A;
  uint16_t battery_max_discharge_A;
  String wifi_ssid;
  String wifi_password;
};

#endif
