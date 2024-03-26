#ifndef __SOC_SCALING_H__
#define __SOC_SCALING_H__

class ScaledSoc {
 private:
  float min_real_soc, max_real_soc, real_soc, scaled_soc;
  bool soc_scaling_active;
  bool real_soc_initialized = false;

 public:
  ScaledSoc() : min_real_soc(20.0f), max_real_soc(80.0f), soc_scaling_active(true) {}
  ScaledSoc(float min_soc, float max_soc, bool scaling_active) {
    min_real_soc = min_soc;
    max_real_soc = max_soc;
    soc_scaling_active = scaling_active;
  }

  float set_real_soc(float soc) {
    real_soc = soc;
    scaled_soc = map_float(real_soc, 0.0f, 100.0f, min_real_soc, max_real_soc);
    real_soc_initialized = true;
    return scaled_soc;
  }

  float get_scaled_soc(void) {
    if (real_soc_initialized) {
      return scaled_soc;
    } else {
      return -1.0f;
    }
  }

  float get_real_soc(void) {
    if (real_soc_initialized) {
      return real_soc;
    } else {
      return -1.0f;
    }
  }

  float get_soc(void) {
    if (real_soc_initialized) {
      if (soc_scaling_active) {
        return scaled_soc;
      } else {
        return real_soc;
      }
    } else {
      return -1.0f;
    }
  }
};

#endif
