#ifndef __PMSIS_BACKEND_NATIVE_TYPES_H__
#define __PMSIS_BACKEND_NATIVE_TYPES_H__

#include "FreeRTOS_util.h"


typedef TaskHandle_t __os_native_task_t;

#define PI_PLATFORM_OTHER  ( 0 )
#define PI_PLATFORM_FPGA   ( 1 )
#define PI_PLATFORM_RTL    ( 2 )
#define PI_PLATFORM_GVSOC  ( 3 )
#define PI_PLATFORM_BOARD  ( 4 )

/* Compat. */
#define ARCHI_PLATFORM_OTHER PI_PLATFORM_OTHER
#define ARCHI_PLATFORM_FPGA  PI_PLATFORM_FPGA
#define ARCHI_PLATFORM_RTL   PI_PLATFORM_RTL
#define ARCHI_PLATFORM_GVSOC PI_PLATFORM_GVSOC
#define ARCHI_PLATFORM_BOARD PI_PLATFORM_BOARD

#endif
