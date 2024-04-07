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
 * Parameter: CAN_BAUD_RATE
 * Description:
 * Defines the baud rate for battery CAN.
 * 
*/
#define CAN_RX_QUEUE_SIZE 10
#define CAN_BAUD_RATE CAN_SPEED_500KBPS

/** TASKS
 * 
 * Parameter: TASK_CORE_PRIO
 * Description:
 * Defines the priority of core functionality (CAN, Modbus, etc)
 * 
 * Parameter: TASK_CONNECTIVITY_PRIO
 * Description:
 * Defines the priority of various wireless functionality (TCP, MQTT, etc)
*/
#define TASK_CORE_PRIO 4
#define TASK_CONNECTIVITY_PRIO 3

/** MAX AMOUNT OF CELLS 
 * 
 * Parameter: MAX_AMOUNT_CELLS
 * Description:
 * Basically the length of the array used to hold individual cell voltages
*/
#define MAX_AMOUNT_CELLS 192

/** LED MODE 
 * 
 * Parameter: LED_MODE_DEFAULT
 * Description:
 * The default LED mode. Can be CLASSIC (slow up/down ramp) or FLOW (slow ramp up/down depending on flow of energy)
 * 
 * Parameter: LED_PERIOD_MS
 * Description:
 * The period of whatever LED mode is active. If CLASSIC, then a ramp up and ramp down will finish in
 * LED_PERIOD_MS milliseconds
*/
#define LED_MODE_DEFAULT HEARTBEAT
#define LED_PERIOD_MS 3000

#endif
