#ifndef _SYSTEM_SETTINGS_H_
#define _SYSTEM_SETTINGS_H_

/** SW VERSION
 * 
 * Parameter: SW_VERSION_STRING
 * Description:
 * Current SW version. Update when necessary
*/
#define SW_VERSION_STRING "6.0.0"

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
 * The default LED mode. Available modes:
 * CLASSIC   - slow up/down ramp
 * FLOW      - slow ramp up or down depending on flow of energy
 * HEARTBEAT - Heartbeat-like LED pattern that reacts to the system state with color and BPM
 * 
 * Parameter: LED_PERIOD_MS
 * Description:
 * The period of whatever LED mode is active. If CLASSIC, then a ramp up and ramp down will finish in
 * LED_PERIOD_MS milliseconds
*/
#define LED_MODE_DEFAULT HEARTBEAT
#define LED_PERIOD_MS 3000

/** MODBUS
 * 
 * Parameter: MB_RTU_NUM_VALUES
 * Description:
 * Number of uint16_t's that the Modbus will keep
 * 
 * Parameter: MB_MAX_POWER
 * Description:
 * Modbus protocols have a limit to the power level.
*/

#define MB_RTU_NUM_VALUES 30000
#define MB_MAX_POWER 40960  //BYD Modbus specific value

#endif
