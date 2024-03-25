#ifndef _DATALAYER_H_
#define _DATALAYER_H_

#include <map>
#include <string>
#include <variant>
#include "../../../system_include.h"

class DataLayer {
 public:
  float SOC;
};

extern DataLayer datalayer;

#endif
