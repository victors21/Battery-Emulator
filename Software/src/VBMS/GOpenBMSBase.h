#include "../include.h"
#include <vector>

template <typename _CountofType, size_t _SizeOfArray>
char (*__countof_helper(_CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
#define _countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)

class GOpenBMSBase {
 public:
  using cell_mv_t = std::vector<uint16_t>;
  GOpenBMSBase(size_t batCapacity_Wh, size_t cellNum);
  uint16_t GetCellDelta_mV();
  inline cell_mv_t& GetCellsData(){return cells_mV;}
  inline uint16_t GetSoC() { return wSOC; }
  inline uint16_t GetTotalCharged_Wh() { return dwTotalCharged_Wh; }
  inline uint16_t GetTotalDisCharged_Wh() { return dwTotalDischaged_Wh; }
  inline uint16_t GetCellMin_mV() { return wMinCellVolts_mV; }
  inline uint16_t GetCellMin_mVNum() { return wMinCellVoltsNum; }
  inline uint16_t GetCellMax_mV() { return wMaxCellVolts_mV; }
  inline uint16_t GetCellMax_mVNum() { return wMaxCellVoltsNum; }
  inline uint16_t GetLeftCapacity_Wh() { return dwTotalCapacityLeft_Wh; }
  inline uint16_t GetTotalCapacity_Wh() { return dwTotalCapacity_Wh; }
  virtual int16_t GetMinTemp() = 0;
  virtual int16_t GetMaxTemp() = 0;
  virtual uint16_t GetBatVotls_dV() = 0;
  virtual int16_t GetBatCurrent_dA() = 0;
  virtual void canData(CAN_frame_t& rx_frame) = 0;
  virtual bool isActive() = 0;
  uint16_t GetMaxChargeCurrent_dA();
  uint16_t GetMaxDisChargeCurrent_dA();
 protected:
  cell_mv_t cells_mV;
  uint16_t wMinCellVolts_mV;
  uint16_t wMinCellVoltsNum;
  uint16_t wMaxCellVolts_mV;
  uint16_t wMaxCellVoltsNum;
  uint16_t wSOC;
  uint32_t dwTotalCharged_Wh;
  uint32_t dwTotalDischaged_Wh;
  uint32_t dwTotalCapacity_Wh;
  uint32_t dwTotalCapacityLeft_Wh;
  uint16_t wMaxAllowedChargeCurrent_dA;
  uint16_t wMaxAllowedDisChargeCurrent_dA;
};
