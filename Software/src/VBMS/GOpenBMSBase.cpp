#include "GOpenBMSBase.h"

GOpenBMSBase::GOpenBMSBase(size_t batCapacity_Wh, size_t cellNum) : cells_mV(cellNum) {
    wMinCellVolts_mV = UINT16_MAX;
    wMaxAllowedChargeCurrent_dA = wMaxAllowedDisChargeCurrent_dA = wMaxCellVolts_mV = 0;
    wMinCellVoltsNum = wMaxCellVoltsNum = UINT16_MAX;
    dwTotalDischaged_Wh = dwTotalCharged_Wh = 0;
    wSOC = 0;
    dwTotalCapacityLeft_Wh = dwTotalCapacity_Wh = static_cast<uint32_t>(batCapacity_Wh);
  }

  uint16_t GOpenBMSBase::GetCellDelta_mV() {
    if (isActive()) {
      return (wMaxCellVolts_mV - wMinCellVolts_mV);
    }
    return 0;
  }

  uint16_t GOpenBMSBase::GetMaxChargeCurrent_dA() {
    return wMaxAllowedChargeCurrent_dA;
  }

  uint16_t GOpenBMSBase::GetMaxDisChargeCurrent_dA() {
    return wMaxAllowedDisChargeCurrent_dA;
  }
