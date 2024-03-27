#ifndef _UTILS_H_
#define _UTILS_H_

#include "debug_handler.h"
#include "events.h"
#include "mapping.h"
#include "settings.h"
#include "soc_scaling.h"
#include "timer.h"
#include "watchdog.h"

enum bms_status_enum { STANDBY = 0, INACTIVE = 1, DARKSTART = 2, ACTIVE = 3, FAULT = 4, UPDATING = 5 };
enum battery_chemistry_enum { NCA, NMC, LFP };
enum led_color { GREEN, YELLOW, RED, BLUE };

#endif
