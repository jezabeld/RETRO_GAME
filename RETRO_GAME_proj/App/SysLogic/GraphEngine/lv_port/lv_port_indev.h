/**
 * @file lv_port_indev.h
 *
 */

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

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
typedef struct {
    lv_obj_t ** objects;
    uint8_t count;
    lv_obj_t * focus_obj;
} screen_setup_t;

typedef enum {
    SCREEN_MAIN_MENU,
    SCREEN_PAUSED
} screen_type_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_port_indev_init(void);
void lv_port_indev_btn_a_pressed(void);
void lv_port_indev_btn_b_pressed(void);
void lv_port_indev_btn_c_pressed(void);
void lv_port_indev_btn_d_pressed(void);
void lv_port_indev_clear_buttons(void);
lv_group_t * lv_port_indev_get_group(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_INDEV_H*/