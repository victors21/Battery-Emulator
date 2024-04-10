#ifndef _TYPES_H_
#define _TYPES_H_

typedef enum { COMM_NONE, COMM_SERIAL, COMM_CAN, COMM_MODBUS } COMMUNICATION_TYPE;

class ExecutionInterface {
 public:
  COMMUNICATION_TYPE comm_type;
  virtual void init(void) = 0;      // Pure virtual function for initialization
  virtual void exe(void) = 0;       // Pure virtual function for execution
  virtual ~ExecutionInterface() {}  // Virtual destructor for proper cleanup
};

#endif
