#include "ChevroletBoltBMS.h"

ChevroletBoltBMS::ChevroletBoltBMS() : GOpenBMSBase(nominal_bat_capacity_Wh, bat_cells_num) {
  wMinCellVolts_mV = static_cast<uint16_t>(cellSocMap[_countof(cellSocMap) - 1].x * 1000.0);
  wMaxCellVolts_mV = static_cast<uint16_t>(cellSocMap[0].x * 1000.0);
  wCellsRounds = dwFinishedCellsRounds = dwTotalChargedDischagedTime = dwTotalCharged_Wms = dwTotalDischaged_Wms = 0;
  memset(raw_cell_data, 0, sizeof(raw_cell_data));
  memset(&lstTemps, 0, sizeof(lstTemps));
  memset(&lstVoltCur, 0, sizeof(lstVoltCur));
  memset(&_bms_state, 0, sizeof(_bms_state));
  wStateMask = eSystemState::starting;
  dwOvercurrentTimer = dwLastCanMessageTime = dwReEnableContactorAfter = 0;
#ifdef DEBUG_VIA_USB
  dwTotalCharged_Wh = 16432949;
#endif
}

int16_t ChevroletBoltBMS::GetMinTemp() {
  if (isActive()) {
    int8_t temperature_min_dC = INT8_MAX;
    for (uint8_t i = 1; i < 7; i++) {
      auto val = lstTemps.GetTemp(i);
      if (temperature_min_dC > val)
        temperature_min_dC = val;
    }
    return temperature_min_dC * 10;
  }
  return 0;
}

int16_t ChevroletBoltBMS::GetMaxTemp() {
  if (isActive()) {
    int8_t temperature_max_dC = INT8_MIN;
    for (uint8_t i = 1; i < 7; i++) {
      auto val = lstTemps.GetTemp(i);
      if (temperature_max_dC < val)
        temperature_max_dC = val;
    }
    return temperature_max_dC * 10;
  }

  return 0;
}

uint16_t ChevroletBoltBMS::GetBatVotls_dV() {
  if (isActive()) {
    return lstVoltCur.GetBatVolt();
  }
  return 0;
}

int16_t ChevroletBoltBMS::GetBatCurrent_dA() {
  if (isActive()) {
    auto curr = lstVoltCur.GetBatAmps25AMax();
    if (curr > 200 || curr < 200)
      return lstVoltCur.GetBatAmps650AMax();
    return curr;
  }
  return 0;
}

bool ChevroletBoltBMS::isActive() {
  return IsStateBitSet(power_on_handled) && dwFinishedCellsRounds > 0;
}

void ChevroletBoltBMS::SetStateBit(const bms_state_t& newStateMask) {
  auto wOldSate = wStateMask;
  wStateMask |= newStateMask;
  if (IsBitWasSet(wOldSate, wStateMask, newStateMask))
    NotifyStateChange(wOldSate, wStateMask);
}

void ChevroletBoltBMS::AlterStateBit(const bms_state_t& alterStateMask) {
  if (IsStateBitSet(alterStateMask)) {
    ClearStateBit(alterStateMask);
  } else
    SetStateBit(alterStateMask);
}

void ChevroletBoltBMS::ClearStateBit(const bms_state_t& newStateMask) {
  auto wOldSate = wStateMask;
  wStateMask = (wStateMask & ~newStateMask);
  if (IsBitWasCleared(wOldSate, wStateMask, newStateMask))
    NotifyStateChange(wOldSate, wStateMask);
}

void ChevroletBoltBMS::NotifyStateChange(const bms_state_t& OldStateMask, const bms_state_t& newStateMask) {
  if (mStateChangedCB)
    mStateChangedCB(OldStateMask, newStateMask);
}
float conversion_rate_to_mv = 1.65f;
void ChevroletBoltBMS::CellsUpdateRoundFinished() {
  if (wCellsRounds >= cells_mV.size())  // if we got all the data then recalc deltas
  {
    wMinCellVolts_mV = UINT16_MAX;
    wMaxCellVolts_mV = 0;
    dwTotalCellBat_mV = 0;
    auto maxCellsNum = cells_mV.size();
    for (uint8_t cellPtrNum = 0; cellPtrNum < _countof(raw_cell_data); cellPtrNum++) {
      if ((cellPtrNum * 3) >= maxCellsNum)
        break;
      auto pInfo = &raw_cell_data[cellPtrNum];
      for (uint8_t i = 0; i < 3; i++) {
        auto cellNum = cellPtrNum * 3 + i;
        if (cellNum >= maxCellsNum)
          break;
        auto& cell_mV = cells_mV[cellNum];
        dwTotalCellBat_mV += cell_mV;
        if (wMinCellVolts_mV > cell_mV) {
          wMinCellVolts_mV = cell_mV;
          wMinCellVoltsNum = cellNum;
        }
        if (wMaxCellVolts_mV < cell_mV) {
          wMaxCellVolts_mV = cell_mV;
          wMaxCellVoltsNum = cellNum;
        }
      }
    }
    dwFinishedCellsRounds++;
    if (!IsStateBitSet(power_on_handled)) {  // START AFTER POWER ON
      auto vBat = lstVoltCur.GetBatVolt() / 10.0;
      auto avgCell_mV = static_cast<float>(vBat / maxCellsNum * 1000.0f);
      auto avgCell_mVc = static_cast<float>(dwTotalCellBat_mV / maxCellsNum);
      auto vDelta = avgCell_mV / avgCell_mVc;
      conversion_rate_to_mv *= vDelta;
      RecalcSocValueForOpenCircuit();
      SetStateBit(power_on_handled);
      OpenAndCloseContactorByTime(2000);  // Turn on contactor
    } else {
      if (IsStateBitSet(cell_overvoltage) && wMaxCellVolts_mV <= max_cell_voltage_mV) {
        ClearStateBit(cell_overvoltage);
      }
      if (IsStateBitSet(cell_undervoltage) && wMinCellVolts_mV >= min_cell_voltage_mV) {
        ClearStateBit(cell_undervoltage);
      }
      auto ldwTotalCharged_Wh = dwTotalCharged_Wms / 360000000;
      if (ldwTotalCharged_Wh) {
        dwTotalCharged_Wh += ldwTotalCharged_Wh;
        if (dwTotalCapacityLeft_Wh < dwTotalCapacity_Wh) {
          dwTotalCapacityLeft_Wh += ldwTotalCharged_Wh;
          RecalcSocValueForCloseCircuit(true);
        }
        dwTotalCharged_Wms = 0;
      }
      auto ldwTotalDischaged_Wh = dwTotalDischaged_Wms / 360000000;
      if (ldwTotalDischaged_Wh) {
        dwTotalDischaged_Wh += ldwTotalDischaged_Wh;
        if (dwTotalCapacityLeft_Wh > 0) {
          dwTotalCapacityLeft_Wh -= ldwTotalDischaged_Wh;
          RecalcSocValueForCloseCircuit(false);
        }
        dwTotalDischaged_Wms = 0;
      }
    }
    RecalcChargeDischargeCurrent();
  }
  wCellsRounds = 0;
}

void ChevroletBoltBMS::RecalcChargeDischargeCurrent() {
  uint16_t Current1C = nominal_bat_capacity_Ah;
  wMaxAllowedChargeCurrent_dA = Current1C / 2 * 10;
  wMaxAllowedDisChargeCurrent_dA = Current1C * 10;
}

void ChevroletBoltBMS::CellVoltageProtectionCheck(uint8_t CellPtrIndex, _can_cell_data* pInfo) {
  auto maxCellsNum = cells_mV.size();
  for (uint8_t i = 0; i < 3; i++) {
    auto cellNum = CellPtrIndex + i;
    if (cellNum < maxCellsNum) {
      auto cell_mV = pInfo->getCellVoltage(i);
      cells_mV[cellNum] = cell_mV;
      if (cell_mV > max_cell_voltage_mV) {
        SetStateBit(eSystemState::cell_overvoltage);
      } else if (cell_mV < min_cell_voltage_mV) {
        SetStateBit(eSystemState::cell_undervoltage);
      }
      wCellsRounds++;
    } else
      break;
  }
}

void ChevroletBoltBMS::RecalcSocValueForCloseCircuit(bool bCharging) {
  if (IsStateBitSet(bat_contactor_enabled)) {
    if (dwTotalCapacityLeft_Wh > dwTotalCapacity_Wh)
      dwTotalCapacityLeft_Wh = dwTotalCapacity_Wh;
    else if (dwTotalCapacityLeft_Wh < 0)
      dwTotalCapacityLeft_Wh = 0;
    auto dNewSoc = ((double)dwTotalCapacityLeft_Wh / (double)dwTotalCapacity_Wh) * 100.0;
    if (dNewSoc != 0 && dNewSoc != 100) {
      auto OCVolts = CalcOCVoltageBySoc(dNewSoc) * (double)cells_mV.size() * 1000.0;
      if ((bCharging && dwTotalCellBat_mV >= OCVolts) || (!bCharging && dwTotalCellBat_mV <= OCVolts))
        wSOC = static_cast<uint16_t>(dNewSoc * 100.0);
    } else
      wSOC = static_cast<uint16_t>(dNewSoc * 100.0);
  }
}

void ChevroletBoltBMS::RecalcSocValueForOpenCircuit() {
  if (!IsStateBitSet(bat_contactor_enabled)) {
    wSOC = static_cast<uint16_t>(
        CalcSocByOCVoltage(((((float)lstVoltCur.GetBatVolt() / (float)cells_mV.size())) / 10.0)) * 100.0);
    dwTotalCapacityLeft_Wh = (dwTotalCapacity_Wh * wSOC) / 10000;
  }
}

double ChevroletBoltBMS::CalcSocByOCVoltage(double cell_mV) {
  for (size_t i = 0; i < _countof(cellSocMap); i++) {
    if (cellSocMap[i].x >= cell_mV) {  // found
      if (i == 0)
        return cellSocMap[i].y;
      else {
        auto& curInfo = cellSocMap[i];
        auto& prevInfo = cellSocMap[(i - 1)];
        return (prevInfo.y + ((curInfo.y - prevInfo.y) * ((cell_mV - prevInfo.x) / (curInfo.x - prevInfo.x))));
      }
    }
  }
  return cellSocMap[(_countof(cellSocMap) - 1)].y;
}

double ChevroletBoltBMS::CalcOCVoltageBySoc(double soc) {
  for (size_t i = 0; i < _countof(cellSocMap); i++) {
    if (cellSocMap[i].y >= soc) {  // found
      if (i == 0)
        return cellSocMap[i].x;
      else {
        auto& curInfo = cellSocMap[i];
        auto& prevInfo = cellSocMap[(i - 1)];
        return (prevInfo.x + ((curInfo.x - prevInfo.x) * ((soc - prevInfo.y) / (curInfo.y - prevInfo.y))));
      }
    }
  }
  return cellSocMap[(_countof(cellSocMap) - 1)].y;
}

uint16_t ChevroletBoltBMS::GetSoh_dP() {
  return static_cast<uint16_t>(10000.0 - ((double)max_recharge_cycles_capacity_closs *
                                          (((double)dwTotalCharged_Wh / (double)dwTotalCapacity_Wh) /
                                           (double)max_recharge_cycles_capacity_closs)));
}

uint32_t ChevroletBoltBMS::CalcMaxCharge_W() {
  return static_cast<uint32_t>((((double)dwTotalCellBat_mV / 1000.0) * (double)GetMaxChargeCurrent_dA()) / 10.0);
}

uint32_t ChevroletBoltBMS::CalcMaxDisCharge_W() {
  return static_cast<uint32_t>((((double)dwTotalCellBat_mV / 1000.0) * (double)GetMaxDisChargeCurrent_dA()) / 10.0);
}

void ChevroletBoltBMS::BatVoltAndCurrentCheck(const uint16_t& Volts, const int16_t& Amps) {
  const uint16_t bat_overvoltage_value = nominal_max_bat_voltage_mV / 100;
  const uint16_t bat_undervoltage_value = nominal_min_bat_voltage_mV / 100;
  if (Volts > 0) {
    if (Volts > bat_overvoltage_value) {
      SetStateBit(bat_overvoltage);
    } else if (Volts < bat_undervoltage_value) {
      SetStateBit(bat_undervoltage);
    } else {
      if (IsStateBitSet(bat_overvoltage)) {
        ClearStateBit(bat_overvoltage);
      }
      if (IsStateBitSet(bat_undervoltage)) {
        ClearStateBit(bat_undervoltage);
      }
    }
  }
  if (Amps > 0)  // Charging
  {
    auto maxCharge_dA = GetMaxChargeCurrent_dA();
    if (Amps > maxCharge_dA) {          // If charge overcurrent
      if ((Amps * 2) < maxCharge_dA) {  // if  up to 2x times
        SetStateBit(bat_charge_overcurrent_up_to_2x_5_seconds);
        dwOvercurrentTimer = millis() + 5000;
      } else {
        SetStateBit(bat_charge_overcurrent_stop_charge);
      }
    } else {
      dwOvercurrentTimer = 0;
      ClearStateBit(bat_charge_overcurrent_up_to_2x_5_seconds | bat_charge_overcurrent_stop_charge);
    }
  } else if (Amps < 0) {  // DisCharging
    auto maxDisCharge_dA = GetMaxDisChargeCurrent_dA();
    auto DischargeCurrent = static_cast<uint16_t>(Amps * -1);
    if (DischargeCurrent > maxDisCharge_dA) {          // If DisCharge overcurrent
      if ((DischargeCurrent * 2) < maxDisCharge_dA) {  // if  up to 2x times
        SetStateBit(bat_discharge_overcurrent_up_to_2x_5_seconds);
        dwOvercurrentTimer = millis() + 5000;
      } else {
        SetStateBit(bat_discharge_overcurrent_stop_discharge);
      }
    } else {
      dwOvercurrentTimer = 0;
      ClearStateBit(bat_discharge_overcurrent_up_to_2x_5_seconds | bat_discharge_overcurrent_stop_discharge);
    }
  }
}

bool ChevroletBoltBMS::ProcessCanData(CAN_frame_t& rx_frame) {
  if (!IsStateBitSet(process_can_messges_allowed)) {
    return false;
  }
  switch (rx_frame.MsgID) {
    case 0x200:
    case 0x202:
    case 0x204:
    case 0x206:
      //case 0x208:
      {
        auto pInfo = reinterpret_cast<_can_cell_data*>(rx_frame.data.u8);
        if (pInfo->isOperate()) {
          auto CellPtrIndex = pInfo->CalcCellIndex(rx_frame.MsgID);
          CellVoltageProtectionCheck(CellPtrIndex, pInfo);
          auto rawCellDataIndex = CellPtrIndex / 3;
          if (rawCellDataIndex < _countof(raw_cell_data)) {
            memcpy(&raw_cell_data[rawCellDataIndex], rx_frame.data.u8, sizeof(_can_cell_data));
          }
          if (CellPtrIndex == 0) {  // new cells round
            CellsUpdateRoundFinished();
          }
        }
        return true;
      }
    case 0x216: {
      memcpy(&lstVoltCur, rx_frame.data.u8, sizeof(lstVoltCur));
      auto CurTime = millis();
      if (isActive()) {
        auto Amps = GetBatCurrent_dA();
        auto Volts = lstVoltCur.GetBatVolt();
        BatVoltAndCurrentCheck(Volts, Amps);
        RecalcSocValueForOpenCircuit();
        auto dwDeltaTime = (CurTime - dwTotalChargedDischagedTime);
        if (dwDeltaTime) {
          auto energyPortion = Volts * Amps * static_cast<int>(dwDeltaTime);
          if (energyPortion > 0) {
            dwTotalCharged_Wms += energyPortion;
          } else if (energyPortion < 0) {
            dwTotalDischaged_Wms += energyPortion * -1;
          }
          dwTotalChargedDischagedTime = CurTime;
        }
      } else {
        dwTotalChargedDischagedTime = CurTime;
      }
      return true;
    }
    case 0x302: {
      memcpy(&lstTemps, rx_frame.data.u8, sizeof(lstTemps));
      return true;
    }
    case 0x20c: {
      memcpy(&_bms_state, rx_frame.data.u8, sizeof(_bms_state));
      return true;
    }
  }
  return false;
}

uint8_t ChevroletBoltBMS::IsBitChanged(const bms_state_t& OldState, const bms_state_t& NewState,
                                       const bms_state_t& CheckMask) {
  if (IsBitWasSet(OldState, NewState, CheckMask))
    return 1U;
  if (IsBitWasCleared(OldState, NewState, CheckMask))
    return 2U;
  return 0;
}

bool ChevroletBoltBMS::IsBitWasSet(const bms_state_t& OldState, const bms_state_t& NewState,
                                   const bms_state_t& CheckMask) {
  return ((OldState & CheckMask) != CheckMask && (NewState & CheckMask) == CheckMask);
}

bool ChevroletBoltBMS::IsBitWasCleared(const bms_state_t& OldState, const bms_state_t& NewState,
                                       const bms_state_t& CheckMask) {
  return ((OldState & CheckMask) == CheckMask && (NewState & CheckMask) != CheckMask);
}

void ChevroletBoltBMS::Loop(const uint32_t& now) {
  if (!IsStateBitSet(becm_can_timeout) && (now - dwLastCanMessageTime) >= bcem_can_timed_out_time) {
    SetStateBit(becm_can_timeout);
    ClearStateBit(power_on_handled);
  }
  if (dwReEnableContactorAfter && now >= dwReEnableContactorAfter) {
    dwReEnableContactorAfter = 0;
    SetStateBit(try_enable_contactor);
  }
  if (dwOvercurrentTimer && now >= dwOvercurrentTimer) {
    dwOvercurrentTimer = 0;
    if (IsStateBitSet(bat_charge_overcurrent_up_to_2x_5_seconds)) {
      SetStateBit(bat_charge_overcurrent_stop_charge);
      ClearStateBit(bat_charge_overcurrent_up_to_2x_5_seconds);
    }
    if (IsStateBitSet(bat_discharge_overcurrent_up_to_2x_5_seconds)) {
      SetStateBit(bat_discharge_overcurrent_stop_discharge);
      ClearStateBit(bat_discharge_overcurrent_up_to_2x_5_seconds);
    }
  }
}

void ChevroletBoltBMS::OpenAndCloseContactorByTime(uint32_t ms) {
  ClearStateBit(bat_contactor_enabled);
  dwReEnableContactorAfter = (millis() + ms);
}

void ChevroletBoltBMS::canData(CAN_frame_t& rx_frame) {
  if (ProcessCanData(rx_frame)) {
    dwLastCanMessageTime = millis();
    if (IsStateBitSet(becm_can_timeout))
      ClearStateBit(becm_can_timeout);
  }
}

uint16_t _can_cell_data::getCellVoltage(uint8_t idx) {
  switch (idx) {
    case 0:
    case 1: {
      auto v = static_cast<uint16_t>(val[idx].le() & 0xFFF);
      return static_cast<uint16_t>(static_cast<float>(v) * conversion_rate_to_mv);
    }
    case 2: {
      auto v = static_cast<uint16_t>((val[idx].le() >> 3) & 0xFFF);
      return static_cast<uint16_t>(static_cast<float>(v) * conversion_rate_to_mv);
    }
  }
  return 0;
}
