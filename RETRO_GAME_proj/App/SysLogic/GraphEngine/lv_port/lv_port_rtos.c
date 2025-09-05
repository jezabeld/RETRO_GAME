/*
 * lv_port_rtos.c
 *
 *  Created on: Aug 28, 2025
 *      Author: jez
 */

#include <string.h>
#include "lv_port_rtos.h"
#include "lvgl.h"

void vApplicationTickHook(void)
{
    lv_tick_inc(1);  // si tu tick de RTOS no es 1 ms: lv_tick_inc(portTICK_PERIOD_MS);
}
void *pvPortRealloc(void *mem, size_t newsize)
{
    if (newsize == 0) {
        vPortFree(mem);
        return NULL;
    }

    void *p;
    p = pvPortMalloc(newsize);
    if (p) {
        /* zero the memory */
        if (mem != NULL) {
            memcpy(p, mem, newsize);
            vPortFree(mem);
        }
    }
    return p;
}

