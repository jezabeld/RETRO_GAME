/**
 * @file lv_port_indev.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev.h"
#include "lvgl.h"
#include "main.h"
#include "InputDrv.h"
#include "UARTDrv.h"
#include <stdlib.h>
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void navigation_input_init(void);
static void navigation_input_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static uint32_t get_navigation_key(void);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_joystick;
static lv_group_t * input_group = NULL;

static volatile bool btn_a_pressed = false;
static volatile bool btn_b_pressed = false;
static volatile bool btn_c_pressed = false;
static volatile bool btn_d_pressed = false;

/* Joystick direction state variables */
static volatile bool joy_up_pressed = false;
static volatile bool joy_down_pressed = false;
static volatile bool joy_left_pressed = false;
static volatile bool joy_right_pressed = false;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    /**
     * Here you will find example implementation of input devices supported by LvGL:
     *  - Touchpad
     *  - Mouse (with cursor support)
     *  - Keypad (supports GUI usage only with key)
     *  - Encoder (supports GUI usage only with: left, right, push)
     *  - Button (external buttons to press points on the screen)
     *
     *  The `..._read()` function are only examples.
     *  You should shape them according to your hardware
     */

    navigation_input_init();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the navigation input device (joystick + buttons)
 */
static void navigation_input_init(void)
{
    static lv_indev_drv_t indev_drv;

    /*Initialize the joystick input device driver*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = navigation_input_read;

    /*Register the driver in LVGL and save the created input device object*/
    indev_joystick = lv_indev_drv_register(&indev_drv);
    
    /*Create an input group for keypad navigation*/
    input_group = lv_group_create();
    lv_group_set_default(input_group);
    
    /*Associate the input device with the group*/
    lv_indev_set_group(indev_joystick, input_group);
}

/**
 * Read navigation input state (joystick + buttons) for LVGL
 * @param indev_drv pointer to the related input device driver
 * @param data store the key data here
 */
static void navigation_input_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    uint32_t key = get_navigation_key();
    
    if(key != 0) {
        data->state = LV_INDEV_STATE_PR;
        data->key = key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/**
 * Get the current navigation key from joystick and buttons
 * @return LVGL key code or 0 if no key is pressed
 */
static uint32_t get_navigation_key(void)
{
    uint32_t key = 0;

    /* Check buttons first (higher priority) */
    if(btn_a_pressed) {
        key = LV_KEY_ENTER;  /* Button A = Enter/Select */
    }
    else if(btn_b_pressed) {
        key = LV_KEY_ESC;    /* Button B = Escape/Back */
    }
    else if(btn_c_pressed) {
        /* Button C = Game-specific action (no menu navigation) */
        key = 0;  /* Ignore in menu, will be handled by game logic later */
    }
    else if(btn_d_pressed) {
        /* Button D = Game-specific action (no menu navigation) */
        key = 0;  /* Ignore in menu, will be handled by game logic later */
    }
    /* Check joystick directions - processed by inputProcessor */
    else if(joy_up_pressed) {
        key = LV_KEY_PREV;  /* Up = Previous element in group */
    }
    else if(joy_down_pressed) {
        key = LV_KEY_NEXT;  /* Down = Next element in group */
    }
    else if(joy_left_pressed) {
        key = LV_KEY_LEFT;
    }
    else if(joy_right_pressed) {
        key = LV_KEY_RIGHT;
    }

    return key;
}

/**
 * Button press handlers - called from main.c EXTI callback
 */
void lv_port_indev_btn_a_pressed(void)
{
    btn_a_pressed = true;
}

void lv_port_indev_btn_b_pressed(void)
{
    btn_b_pressed = true;
}

void lv_port_indev_btn_c_pressed(void)
{
    btn_c_pressed = true;
}

void lv_port_indev_btn_d_pressed(void)
{
    btn_d_pressed = true;
}

/**
 * Joystick direction press handlers - called from UIController
 */
void lv_port_indev_joy_up_pressed(void)
{
    joy_up_pressed = true;
}

void lv_port_indev_joy_down_pressed(void)
{
    joy_down_pressed = true;
}

void lv_port_indev_joy_left_pressed(void)
{
    joy_left_pressed = true;
}

void lv_port_indev_joy_right_pressed(void)
{
    joy_right_pressed = true;
}

/**
 * Clear button and joystick states - should be called periodically
 */
void lv_port_indev_clear_buttons(void)
{
    btn_a_pressed = false;
    btn_b_pressed = false;
    btn_c_pressed = false;
    btn_d_pressed = false;

    joy_up_pressed = false;
    joy_down_pressed = false;
    joy_left_pressed = false;
    joy_right_pressed = false;
}

/**
 * Get the input group for adding UI objects
 * @return pointer to the input group
 */
lv_group_t * lv_port_indev_get_group(void)
{
    return input_group;
}