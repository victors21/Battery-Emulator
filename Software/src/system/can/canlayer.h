#ifndef _CANLAYER_H_
#define _CANLAYER_H_

#include "../../system_include.h"

// These types are copied from ESP32CAN for abstraction purposes

// typedef enum { CAN_NONE, CAN_ESP32CAN, CAN_MCP2515, CAN_MCP2517 } CAN_TYPE;
class CanInterface;
typedef std::unique_ptr<CanInterface> CanInterfacePtr;

typedef enum {
  CAN_BAUDRATE_125 = 125,
  CAN_BAUDRATE_500 = 500,
  CAN_BAUDRATE_1000 = 1000,
} CAN_BAUDRATE;

/**
 * \brief CAN frame format type (standard/extended)
 */
typedef enum {
  CAN_STANDARD_FRAME = 0, /**< Standard frame, using 11 bit identifer. */
  CAN_EXTENDED_FRAME = 1  /**< Extended frame, using 29 bit identifer. */
} CAN_FF_TYPE;

/**
 * \brief CAN channel
*/
typedef enum { CAN_CHANNEL_NONE, CAN_1, CAN_2 } CAN_CHANNEL_ENUM;

/**
 * \brief CAN RTR
 */
typedef enum {
  CAN_NOT_RTR_FRAME = 0, /**< No RTR frame. */
  CAN_RTR_FRAME = 1      /**< RTR frame. */
} CAN_RTR_TYPE;

/** \brief Frame information record type */
typedef union {
  uint32_t U; /**< \brief Unsigned access */
  struct {
    uint8_t DLC : 4;               /**< \brief [3:0] DLC, Data length container */
    unsigned int unknown_2 : 2;    /**< \brief \internal unknown */
    CAN_RTR_TYPE RTR : 1;          /**< \brief [6:6] RTR, Remote Transmission Request */
    CAN_FF_TYPE FF : 1;            /**< \brief [7:7] Frame Format, see# CAN_frame_format_t*/
    unsigned int reserved_24 : 24; /**< \brief \internal Reserved */
  } B;
} CAN_FIR_TYPE;

/** \brief CAN Frame structure */
typedef struct {
  CAN_FIR_TYPE FIR; /**< \brief Frame information record*/
  uint32_t MsgID;   /**< \brief Message ID */
  union {
    uint8_t u8[8];   /**< \brief Payload byte access*/
    uint32_t u32[2]; /**< \brief Payload u32 access*/
    uint64_t u64;    /**< \brief Payload u64 access*/
  } data;
} CAN_FRAME_TYPE;

class CanInterface {
 protected:
  // CAN_CHANNEL_ENUM channel;
 public:
  // CanInterface(CAN_BAUDRATE baudrate);

  virtual void init(void) = 0;
  virtual void transmit(const CAN_FRAME_TYPE& frame) = 0;  // Additional pure virtual function for transmission

  virtual ~CanInterface() {}  // Virtual destructor for proper cleanup
};

class CanUser {
 public:
  CanInterface* can = nullptr;

  CanUser() = default;   // Allow default construction
  virtual ~CanUser() {}  // Virtual destructor for proper cleanup
};

CanInterface* can1_get_instance(CAN_BAUDRATE baudrate);

#endif
