/**
 * @file lv_port_disp.h
 *
 */

#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
/* Initialize low level display driver */
void lv_port_disp_init(void);

/* Logging callback */
void my_log_cb(const char * buf);

#endif /* LV_PORT_DISP_H */
