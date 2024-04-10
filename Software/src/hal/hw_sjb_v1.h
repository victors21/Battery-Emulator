#ifndef _HW_SJB_V1_H_
#define _HW_SJB_V1_H_

#define CAN_1_TYPE CAN_MCP2517
#define CAN_2_TYPE CAN_MCP2517

/* ----- Error checks below, don't change ----- */
#ifndef HW_CONFIGURED
#define HW_CONFIGURED
#else
#error Multiple HW defined! Please select a single HW
#endif

#endif
