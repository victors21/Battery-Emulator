#include "../include.h"
#ifdef CHEVROLET_BOLT_BATTERY
#include "../VBMS/ChevroletBoltBMS.h"
#include "../datalayer/datalayer.h"
#include "../devboard/utils/events.h"
#include "../lib/miwagner-ESP32-Arduino-CAN/CAN_config.h"
#include "../lib/miwagner-ESP32-Arduino-CAN/ESP32CAN.h"
#include "CHEVROLET-BOLT-BATTERY.h"

ChevroletBoltBMS BMS;

void update_values_battery() {

  datalayer.battery.info.max_charge_amp_dA = BMS.GetMaxChargeCurrent_dA();
  datalayer.battery.info.max_discharge_amp_dA = BMS.GetMaxDisChargeCurrent_dA();
  datalayer.battery.status.remaining_capacity_Wh = BMS.GetLeftCapacity_Wh();
  datalayer.battery.status.soh_pptt = BMS.GetSoh_dP();
  datalayer.battery.status.temperature_max_dC = BMS.GetMaxTemp();
  datalayer.battery.status.temperature_min_dC = BMS.GetMinTemp();
  datalayer.battery.status.cell_min_voltage_mV = BMS.GetCellMin_mV();
  datalayer.battery.status.cell_max_voltage_mV = BMS.GetCellMax_mV();
  datalayer.battery.status.voltage_dV = BMS.GetBatVotls_dV();
  datalayer.battery.status.current_dA = BMS.GetBatCurrent_dA();
  auto cells = BMS.GetCellsData();
  memcpy(datalayer.battery.status.cell_voltages_mV, &cells.at(0),
         sizeof(ChevroletBoltBMS::cell_mv_t::value_type) * cells.size());
  datalayer.battery.status.real_soc = BMS.GetSoC();
  datalayer.battery.status.active_power_W =  //Power in watts, Negative = charging batt
      ((datalayer.battery.status.voltage_dV * datalayer.battery.status.current_dA) / 100);
  datalayer.battery.status.max_charge_power_W = BMS.CalcMaxCharge_W();
  datalayer.battery.status.max_discharge_power_W = BMS.CalcMaxDisCharge_W();

#ifdef DEBUG_VIA_USB
auto soc = BMS.GetSoC() / 100.0;
  Serial.printf("BMS State %.4x EventLevel %u Cell Sum %.3f mV Bat %.2f Soc %.2f SocVolt %.2f SOH %.2f\n", BMS.GetState(), get_event_level(),
                BMS.GetTotalCellBat_mV() / 1000.0, BMS.GetBatVotls_dV() / 10.0, soc,BMS.CalcOCVoltageBySoc(soc)*96.0,BMS.GetSoh_dP()/100.0);
#endif
}

void receive_can_battery(CAN_frame_t rx_frame) {
  BMS.canData(rx_frame);
}
void send_can_battery() {
  BMS.Loop(millis());
}

void setup_battery(void) {  // Performs one time setup at startup
#ifdef DEBUG_VIA_USB
  Serial.println("CHEVROLET BOLT battery selected");
#endif
  datalayer.battery.info.chemistry = battery_chemistry_enum::NMC;
  datalayer.battery.info.number_of_cells = ChevroletBoltBMS::bat_cells_num;
  datalayer.battery.info.max_design_voltage_dV = ChevroletBoltBMS::nominal_max_bat_voltage_mV / 100;
  datalayer.battery.info.min_design_voltage_dV = ChevroletBoltBMS::nominal_min_bat_voltage_mV / 100;
  datalayer.battery.info.max_charge_amp_dA = 0;
  datalayer.battery.info.max_discharge_amp_dA = 0;
  datalayer.battery.info.total_capacity_Wh = ChevroletBoltBMS::nominal_bat_capacity_Wh;
  // Temporary disable discharge and charge when starting up
  datalayer.battery.status.max_charge_power_W = 0;
  datalayer.battery.status.max_discharge_power_W = 0;
  datalayer.battery.status.remaining_capacity_Wh = 0;
  datalayer.battery.status.temperature_max_dC = -1500;
  datalayer.battery.status.temperature_min_dC = 1500;
  datalayer.battery.status.voltage_dV = 0;
  datalayer.battery.status.bms_status = STANDBY;
  BMS.StateChangeEvent([](const uint16_t& OldState, const uint16_t& NewState) {
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::power_on_handled)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.println("SET power_on_handled");
#endif
        datalayer.battery.status.bms_status = ACTIVE;
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::bms_logic_error)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.println("SET bms_logic_error");
#endif
        datalayer.battery.status.bms_status = FAULT;
        BMS.ClearStateBit(ChevroletBoltBMS::bat_contactor_enabled);
        break;
      }
      case 2: {
#ifdef DEBUG_VIA_USB
        Serial.println("CLEAR bms_logic_error");
#endif
        datalayer.battery.status.bms_status = ACTIVE;
        if (BMS.isActive() && !BMS.IsStateBitSet(ChevroletBoltBMS::bat_contactor_enabled))
          BMS.OpenAndCloseContactorByTime(2000);  // Waiting for invertor 2000 ms and then enable contactor
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::becm_can_timeout)) {
      case 1: {
        set_event(EVENT_CAN_RX_FAILURE, 0);
#ifdef DEBUG_VIA_USB
        Serial.println("SET EVENT_CAN_RX_FAILURE");
#endif
        BMS.ClearStateBit(ChevroletBoltBMS::bat_contactor_enabled);
        break;
      }
      case 2: {
        clear_event(EVENT_CAN_RX_FAILURE);
#ifdef DEBUG_VIA_USB
        Serial.println("CLEAR EVENT_CAN_RX_FAILURE");
#endif
        if (BMS.isActive() && !BMS.IsStateBitSet(ChevroletBoltBMS::bat_contactor_enabled))
          BMS.OpenAndCloseContactorByTime(2000);  // Waiting for invertor 2000 ms and then enable contactor
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::try_enable_contactor)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.printf("SET try_enable_contactor\n");
#endif
        if (datalayer.system.status.inverter_allows_contactor_closing) {
          BMS.SetStateBit(ChevroletBoltBMS::bat_contactor_enabled);
        } else {
          Serial.printf("try_enable_contactor delayed for 5000ms\n");
          BMS.OpenAndCloseContactorByTime(5000);  // Waiting for invertor 5000 ms and then retry enable contactor
        }
        BMS.ClearStateBit(ChevroletBoltBMS::try_enable_contactor);
        break;
      }
#ifdef DEBUG_VIA_USB
      case 2: {

        Serial.printf("CLEAR try_enable_contactor\n");

        break;
      }
#endif
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::bat_contactor_enabled)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.println("SET bat_contactor_enabled");
#endif
        if (!BMS.IsStateBitSet(ChevroletBoltBMS::bms_logic_error))
          datalayer.system.status.battery_allows_contactor_closing = true;
        break;
      }
      case 2: {
#ifdef DEBUG_VIA_USB
        Serial.println("CLEAR bat_contactor_enabled");
#endif
        datalayer.system.status.battery_allows_contactor_closing = false;
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(
        OldState, NewState, (ChevroletBoltBMS::bat_undervoltage | ChevroletBoltBMS::cell_undervoltage))) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.printf("SET bat_undervoltage|cell_undervoltage\n");
#endif
        set_event(EVENT_BATTERY_DISCHG_STOP_REQ, 0);
        BMS.OpenAndCloseContactorByTime(
            2000);  // Disconnect battary and wait 2000 ms while invertor make reaction to message and stop discharge then re enable contactor
        break;
      }
      case 2: {
#ifdef DEBUG_VIA_USB
        Serial.printf("CLEAR bat_undervoltage|cell_undervoltage\n");
#endif
        clear_event(EVENT_BATTERY_DISCHG_STOP_REQ);
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::cell_undervoltage)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.printf("SET cell_undervoltage\n");
#endif
        set_event(EVENT_CELL_UNDER_VOLTAGE, 0);
        break;
      }
      case 2: {
#ifdef DEBUG_VIA_USB
        Serial.printf("CLEAR cell_undervoltage\n");
#endif
        clear_event(EVENT_CELL_UNDER_VOLTAGE);
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::cell_overvoltage)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.printf("SET cell_overvoltage\n");
#endif
        set_event(EVENT_CELL_OVER_VOLTAGE, 0);
        break;
      }
      case 2: {
#ifdef DEBUG_VIA_USB
        Serial.printf("CLEAR cell_overvoltage\n");
#endif
        clear_event(EVENT_CELL_OVER_VOLTAGE);
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::bat_charge_overcurrent_stop_charge)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.printf("SET EVENT_BATTERY_CHG_STOP_REQ\n");
#endif
        set_event(EVENT_BATTERY_CHG_STOP_REQ, 0);
        break;
      }
      case 2: {
#ifdef DEBUG_VIA_USB
        Serial.printf("CLEAR EVENT_BATTERY_CHG_STOP_REQ\n");
#endif
        clear_event(EVENT_BATTERY_CHG_STOP_REQ);
        break;
      }
    }
    switch (ChevroletBoltBMS::IsBitChanged(OldState, NewState, ChevroletBoltBMS::bat_discharge_overcurrent_stop_discharge)) {
      case 1: {
#ifdef DEBUG_VIA_USB
        Serial.printf("SET EVENT_BATTERY_DISCHG_STOP_REQ\n");
#endif
        set_event(EVENT_BATTERY_DISCHG_STOP_REQ, 0);
        break;
      }
      case 2: {
#ifdef DEBUG_VIA_USB
        Serial.printf("CLEAR EVENT_BATTERY_DISCHG_STOP_REQ\n");
#endif
        clear_event(EVENT_BATTERY_DISCHG_STOP_REQ);
        break;
      }
    }
  });
  BMS.SetStateBit(ChevroletBoltBMS::process_can_messges_allowed | ChevroletBoltBMS::becm_can_timeout);
}

#endif
