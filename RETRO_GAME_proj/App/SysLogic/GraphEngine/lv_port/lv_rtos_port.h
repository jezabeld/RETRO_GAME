/*
 * lv_rtos_port.h
 *
 *  Created on: Aug 28, 2025
 *      Author: jez
 */

#ifndef SYSLOGIC_GRAPHENGINE_LV_PORT_LV_RTOS_PORT_H_
#define SYSLOGIC_GRAPHENGINE_LV_PORT_LV_RTOS_PORT_H_

#include "cmsis_os.h"

void vApplicationTickHook(void);
void *pvPortRealloc(void *mem, size_t newsize);

#endif /* SYSLOGIC_GRAPHENGINE_LV_PORT_LV_RTOS_PORT_H_ */
