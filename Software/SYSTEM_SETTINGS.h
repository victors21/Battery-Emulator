#ifndef _SYSTEM_SETTINGS_H_
#define _SYSTEM_SETTINGS_H_

/** WATCHDOG
 * 
 * Parameter: TASK_BACKGROUND_WATCHDOG_PERIOD
 * Description:
 * Sets the timeout period of the watchdog in seconds
 * 
*/
#define TASK_BACKGROUND_WATCHDOG_PERIOD 5

/** EEPROM
 * 
 * Parameter: EE_SIZE
 * Description:
 * Defines the allocated EE size in bytes
 * 
 * Parameter: EE_BOOT_RESET_START_ADDRESS
 * Description:
 * Defines the watchdog reset information section. This section is
 * used to record information about reset reasons to enable
 * starting the lilygo in a "safe mode", for example in case of
 * reset loops
 * 
 * Parameter: EE_EVENTS_START_ADDRESS
 * Description:
 * Defines the start address of the events section
 * 
*/
#define EE_SIZE 1024
#define EE_EVENTS_START_ADDRESS 64
#define EE_BOOT_RESET_START_ADDRESS 0

/** CAN
 * 
 * Parameter: CAN_RX_QUEUE_SIZE
 * Description:
 * Defines the size of the queue for RX frames
 * 
*/
#define CAN_RX_QUEUE_SIZE 10
#define CAN_BAUD_RATE CAN_SPEED_500KBPS

#endif
