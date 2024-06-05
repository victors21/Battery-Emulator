
#include "GOpenBMSBase.h"

// Usual battary parameter constants which is can not be read from BMS

#ifndef BATTARY_CELLS_NUM
#define BATTARY_CELLS_NUM 96  // e.g. number of cells in battary
#endif
#ifndef BATTARY_TEMPS_NUM
#define BATTARY_TEMPS_NUM 6  // e.g. number of temp sensors in battary
#endif
#ifndef BATTARY_CELL_MAX_VOLTAGE
#define BATTARY_CELL_MAX_VOLTAGE 417  // e.g. 417/10 = 4.17V max cell voltage
#endif
#ifndef BATTARY_CELL_MIN_VOLTAGE
#define BATTARY_CELL_MIN_VOLTAGE 317  // e.g. 317/10 = 3.17V min cell voltage
#endif
#ifndef BATTARY_CELL_MID_VOLTAGE
#define BATTARY_CELL_MID_VOLTAGE 370  // e.g. 370/10 = 3.7V mid cell voltage
#endif
#ifndef BATTARY_MAX_CAP_WH
#define BATTARY_MAX_CAP_WH 60000  // e.g. 60kWh
#endif
#ifndef BATTARY_CHARGE_CURRENT_RATE
#define BATTARY_CHARGE_CURRENT_RATE 12  // e.g. 12/10 = Charge current rate 1.2C
#endif
#ifndef BATTARY_DISHARGE_CURRENT_RATE
#define BATTARY_DISHARGE_CURRENT_RATE 12  // e.g. 12/10 = Discharge current rate 1.2C
#endif

const uint16_t charge_cutoff_voltage = BATTARY_CELLS_NUM * BATTARY_CELL_MAX_VOLTAGE / 10;  // max battary charge voltage
const uint16_t discharge_cutoff_voltage =
    BATTARY_CELLS_NUM * BATTARY_CELL_MIN_VOLTAGE / 10;  // min battary discharge voltage

const uint8_t charge_current_rate = BATTARY_CHARGE_CURRENT_RATE;       // e.g. 12/10 = Charge current rate 1.2C
const uint8_t discharge_current_rate = BATTARY_DISHARGE_CURRENT_RATE;  // e.g. 12/10 = Discharge current  rate 1.2C

const int16_t max_charge_current = (BATTARY_MAX_CAP_WH * BATTARY_CHARGE_CURRENT_RATE) /
                                   (BATTARY_CELL_MID_VOLTAGE * BATTARY_CELLS_NUM);  // e.g. 20 Amps
const int16_t max_discharge_current = (BATTARY_MAX_CAP_WH * BATTARY_DISHARGE_CURRENT_RATE) /
                                      (BATTARY_CELL_MID_VOLTAGE * BATTARY_CELLS_NUM);  // e.g. 20 Amps

#pragma pack(push, 1)
union can_uint16_t {
  uint8_t u8[2];
  uint16_t u16;
  uint16_t le() {
    can_uint16_t v{u8[1], u8[0]};
    return v.u16;
  }
};
union can_int16_t {
  int8_t i8[2];
  int16_t i16;
  int16_t le() {
    can_int16_t v{i8[1], i8[0]};
    return v.i16;
  }
};

struct _can_cell_data {
  enum {
    cell_data_valid_bit = 0x8,
  };
  can_uint16_t val[3];
  uint8_t cell_state;               
  uint8_t reserved;
  // Check if BMS Has normal mesured value, when it's just started this bit is set to zero
  bool isOperate() { return (cell_state & cell_data_valid_bit) == cell_data_valid_bit; }
  uint16_t getCellVoltage(uint8_t idx);
  uint8_t getCellErrorMask() { return static_cast<uint8_t>(((val[0].u8[0] & 0xF) << 8) + (val[1].u8[0])); }
  uint8_t CalcCellIndex(uint32_t msgID) {
    return (((msgID - 0x200) / 2) * 24) + ((((cell_state&0xF0) - 0x10) / 0x20) * 3);
  }
};
struct _can_volt_cur_bat_info {
  uint8_t hvilStatus;
  can_uint16_t batVots;
  can_int16_t batAmpsHiVal;
  can_int16_t batAmpsLoVal;
  //value is *10 (3700 = 370.0)
  uint16_t GetBatVolt() {
    auto v = batVots.le() & 0xFFF;
    return static_cast<uint16_t>(v * 1.25f);
  }
  //value is *10 (150 = 15.0) , invert the sign
  int16_t GetBatAmps25AMax() {
    auto amps = batAmpsLoVal.le();
    return static_cast<int16_t>(amps * 0.225f);
  }
  //value is *10 (150 = 15.0) , invert the sign
  int16_t GetBatAmps650AMax() {
    auto amps = batAmpsHiVal.le();
    return static_cast<int16_t>(amps * 0.225f);
  }
};
struct _can_bat_temperatures_data {
  int8_t temps[8];
  int8_t GetTemp(uint8_t idx) { return static_cast<int8_t>(temps[idx] - 100); }
};
struct _can_startup_data {
    enum eStartupStateBits : uint8_t{
        bms_is_booting = 1,
        bms_is_booted = 2,
    };
    enum eADCStateBits : uint8_t{
        cell_line_1 = 1,
        cell_line_2 = 2,
        cell_line_3 = 4,
    };
  eStartupStateBits startup_state;
  uint8_t reserved;           
  eADCStateBits adc_state;
  uint8_t adc_calibration[3];
  bool isCellLineReady(uint8_t cellNum){
    return isCellLineSet(1<<cellNum);
  }
  bool isCellLineSet(uint8_t cellMask){
    return (adc_state&cellMask) == cellMask;
  }
};
#pragma pack(pop)

class ChevroletBoltBMS : public GOpenBMSBase {
 public:
  enum eSystemState : uint16_t {
    starting,
    power_on_handled,
    process_can_messges_allowed = (power_on_handled << 1),
    becm_can_timeout = (process_can_messges_allowed << 1),
    cell_overvoltage = (becm_can_timeout << 1),
    cell_undervoltage = (cell_overvoltage << 1),
    bat_overvoltage = (cell_undervoltage << 1),
    bat_undervoltage = (bat_overvoltage << 1),
    bat_overcurrent = (bat_undervoltage << 1),
    bat_contactor_enabled = (bat_overcurrent << 1),
    try_enable_contactor = (bat_contactor_enabled << 1),
    enable_contactor_lockd_by_invertor = (try_enable_contactor << 1),
    bms_logic_error = (enable_contactor_lockd_by_invertor << 1),
    bat_charge_overcurrent_up_to_2x_5_seconds = (bms_logic_error << 1),
    bat_charge_overcurrent_stop_charge = (bat_charge_overcurrent_up_to_2x_5_seconds << 1),
    bat_discharge_overcurrent_up_to_2x_5_seconds = (bat_charge_overcurrent_stop_charge << 1),
    bat_discharge_overcurrent_stop_discharge = (bat_discharge_overcurrent_up_to_2x_5_seconds << 1),
    
  };
  enum : uint32_t {
    max_can_cell_rows = 32,
    bat_cells_num = BATTARY_CELLS_NUM,
    bcem_can_timed_out_time = 10000,
    max_cell_voltage_mV = 4200,
    max_bat_voltage_mV = max_cell_voltage_mV * bat_cells_num,
    min_cell_voltage_mV = 2500,
    min_bat_voltage_mV = min_cell_voltage_mV * bat_cells_num,
    nominal_min_cell_voltage_mV = 3167,
    nominal_min_bat_voltage_mV = nominal_min_cell_voltage_mV * bat_cells_num,
    nominal_max_cell_voltage_mV = 4166,
    nominal_max_bat_voltage_mV = nominal_max_cell_voltage_mV * bat_cells_num,
    normal_cell_current_mA = 32100,
    nominal_bat_capacity_Ah = 180,
    mominal_cell_voltage_mV = 3600,
    nominal_bat_capacity_Wh = ((nominal_bat_capacity_Ah * mominal_cell_voltage_mV * bat_cells_num) / 1000),
    max_recharge_cycles_capacity_closs = 10000,
  };
  struct _table_point {
    double x, y;
  };
  const _table_point cellSocMap[21] = {{3.167, 0},  {3.413, 5},  {3.446, 10}, {3.488, 15}, {3.537, 20}, {3.571, 25},
                                       {3.593, 30}, {3.610, 35}, {3.625, 40}, {3.642, 45}, {3.663, 50}, {3.695, 55},
                                       {3.755, 60}, {3.799, 65}, {3.846, 70}, {3.895, 75}, {3.945, 80}, {3.997, 85},
                                       {4.051, 90}, {4.108, 95}, {4.166, 100}};
  using bms_state_t = uint16_t;
  using new_state_cb_t = std::function<void(const bms_state_t& OldState, const bms_state_t& NewState)>;
  ChevroletBoltBMS();
  virtual int16_t GetMinTemp();
  virtual int16_t GetMaxTemp();
  virtual uint16_t GetBatVotls_dV();
  virtual int16_t GetBatCurrent_dA();
  virtual void canData(CAN_frame_t& rx_frame);
  virtual bool isActive();
  void SetStateBit(const bms_state_t& newStateMask);
  void AlterStateBit(const bms_state_t& alterStateMask);
  void ClearStateBit(const bms_state_t& newStateMask);
  void NotifyStateChange(const bms_state_t& OldStateMask, const bms_state_t& newStateMask);
  inline void StateChangeEvent(const new_state_cb_t& f) { mStateChangedCB = f; }
  // Return 1 if bit was set 2 if cleared 0 if not changed
  static uint8_t IsBitChanged(const bms_state_t& OldState, const bms_state_t& NewState, const bms_state_t& CheckMask);
  static bool IsBitWasSet(const bms_state_t& OldState, const bms_state_t& NewState, const bms_state_t& CheckMask);
  static bool IsBitWasCleared(const bms_state_t& OldState, const bms_state_t& NewState, const bms_state_t& CheckMask);
  inline bool IsStateBitSet(const bms_state_t& mask) { return (wStateMask & mask) == mask; }
  void Loop(const uint32_t& now);
  void OpenAndCloseContactorByTime(uint32_t ms);
  inline bms_state_t GetState() { return wStateMask; }
  uint32_t GetTotalCellBat_mV(){return dwTotalCellBat_mV;}
  double CalcSocByOCVoltage(double cell_mV);
  double CalcOCVoltageBySoc(double soc);
  uint16_t GetSoh_dP();
  uint32_t CalcMaxCharge_W();
  uint32_t CalcMaxDisCharge_W();
 protected:
  void CellsUpdateRoundFinished();
  void RecalcChargeDischargeCurrent();
  void CellVoltageProtectionCheck(uint8_t CellPtrIndex, _can_cell_data* pInfo);
  // bCharging = true if battary Charging
  void RecalcSocValueForCloseCircuit(bool bCharging); 
  void RecalcSocValueForOpenCircuit();
  _can_cell_data raw_cell_data[max_can_cell_rows];
  _can_bat_temperatures_data lstTemps;
  _can_volt_cur_bat_info lstVoltCur;
  _can_startup_data _bms_state;
  uint16_t wCellsRounds;
  uint32_t dwFinishedCellsRounds;
  uint32_t dwTotalChargedDischagedTime;
  uint32_t dwTotalCharged_Wms;
  uint32_t dwTotalDischaged_Wms;
  uint32_t dwLastCanMessageTime;
  uint32_t dwReEnableContactorAfter;
  bms_state_t wStateMask;
  new_state_cb_t mStateChangedCB;
  uint32_t dwTotalCellBat_mV;
  uint32_t dwOvercurrentTimer;
  void BatVoltAndCurrentCheck(const uint16_t& Volts, const int16_t& Amps);
  bool ProcessCanData(CAN_frame_t& rx_frame);
};
