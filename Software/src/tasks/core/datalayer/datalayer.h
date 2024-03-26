#ifndef _DATALAYER_H_
#define _DATALAYER_H_

#include "../../../system_include.h"
#include "../../../utils/utils.h"

typedef struct {
  /** float */
  float max_design_voltage_V;
  float min_design_voltage_V;

  /** uint32_t */
  uint32_t total_capacity;

  /** uint16_t */
  uint16_t number_of_cells;

  /** Other */
  battery_chemistry_enum chemistry;
} DATALAYER_BATTERY_DATA_TYPE;

typedef struct {
  /** float */
  float temperature_max_C;
  float temperature_min_C;
  float current_A;
  float voltage_V;
  float soh_pct;
  // float soc_real_pct;
  // float soc_scaled_pct;

  /** uint32_t */
  uint32_t remaining_capacity;

  /** int16_t */
  int16_t active_power_W;

  /** uint16_t */
  uint16_t max_discharge_power_W;
  uint16_t max_charge_power_W;
  uint16_t cell_max_voltage_mV;
  uint16_t cell_min_voltage_mV;
  uint16_t cell_voltages_mV[MAX_AMOUNT_CELLS];

  /** Other */
  bms_status_enum bms_status;
  ScaledSoc scaled_soc;
} DATALAYER_BATTERY_STATUS_TYPE;

typedef struct {
  DATALAYER_BATTERY_DATA_TYPE data;
  DATALAYER_BATTERY_STATUS_TYPE status;
} DATALAYER_BATTERY_TYPE;

class DataLayer {
 public:
  DATALAYER_BATTERY_TYPE battery;
};

extern DataLayer datalayer;

#endif
