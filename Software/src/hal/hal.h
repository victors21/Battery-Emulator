#ifndef _HAL_H_
#define _HAL_H_

#include "../../USER_SETTINGS.h"

#if defined(HW_LILYGO)
#include "hw_lilygo.h"
#elif defined(HW_BATTERY_EMULATOR_v0)
#include "HW_BATTERY_EMULATOR_v0.h"
#endif

#if !defined(HW_CONFIGURED)
#error You must select a HW to run on!
#endif

#endif
