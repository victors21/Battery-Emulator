#ifndef __MAPPING_H__
#define __MAPPING_H__

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define CONSTRAIN(val, min, max) (MIN(max, MAX(min, val)))

static inline float map_float(float val, float in_min, float in_max, float out_min, float out_max) {
  if (val <= in_min) {
    return out_min;
  } else if (val > in_max) {
    return out_max;
  } else {
    return (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }
}

#endif
