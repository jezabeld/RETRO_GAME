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
#include <stdlib.h>
#include <stdint.h>

/*********************
 *      DEFINES
 *********************/
#define JOYSTICK_CENTER_X   2048    /* 12-bit ADC center value */
#define JOYSTICK_CENTER_Y   2048    /* 12-bit ADC center value */
#define JOYSTICK_DEADZONE   800     /* Dead zone around center */
#define JOYSTICK_DEBOUNCE_TIME 200  /* Debounce time in ms */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void joystick_init(void);
static void joystick_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static uint32_t joystick_get_key(void);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_indev_t * indev_joystick;
static lv_group_t * input_group = NULL;
extern volatile uint16_t joy_raw[2]; /* From main.c: [0]=X, [1]=Y */

static volatile bool btn_a_pressed = false;
static volatile bool btn_b_pressed = false;
static volatile bool btn_c_pressed = false;
static volatile bool btn_d_pressed = false;

/* Joystick direction state variables */
static volatile uint32_t last_key_time = 0;
static volatile uint32_t last_key = 0;

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

    joystick_init();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Initialize the joystick input device
 */
static void joystick_init(void)
{
    static lv_indev_drv_t indev_drv;

    /*Initialize the joystick input device driver*/
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = joystick_read;

    /*Register the driver in LVGL and save the created input device object*/
    indev_joystick = lv_indev_drv_register(&indev_drv);
    
    /*Create an input group for keypad navigation*/
    input_group = lv_group_create();
    lv_group_set_default(input_group);
    
    /*Associate the input device with the group*/
    lv_indev_set_group(indev_joystick, input_group);
}

/**
 * Get the current key state of the joystick and buttons
 * @param indev_drv pointer to the related input device driver
 * @param data store the key data here
 */
static void joystick_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    uint32_t key = joystick_get_key();
    
    if(key != 0) {
        data->state = LV_INDEV_STATE_PR;
        data->key = key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/**
 * Get the current key pressed (joystick direction or button)
 * @return key code or 0 if no key is pressed
 */
static uint32_t joystick_get_key(void)
{
    uint32_t current_time = HAL_GetTick();
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
    else {
        /* Check joystick directions */
        int16_t x_diff = (int16_t)joy_raw[0] - JOYSTICK_CENTER_X;
        int16_t y_diff = (int16_t)joy_raw[1] - JOYSTICK_CENTER_Y;
        
        /* Determine primary direction (strongest movement) */
        if(abs(x_diff) > JOYSTICK_DEADZONE || abs(y_diff) > JOYSTICK_DEADZONE) {
            if(abs(x_diff) > abs(y_diff)) {
                /* Horizontal movement is stronger */
                if(x_diff > 0) {
                    key = LV_KEY_RIGHT;
                } else {
                    key = LV_KEY_LEFT;
                }
            } else {
                /* Vertical movement is stronger */
                if(y_diff > 0) {
                    key = LV_KEY_NEXT;  /* Down = Next element in group */
                } else {
                    key = LV_KEY_PREV;  /* Up = Previous element in group */
                }
            }
        }
    }
    
    /* Implement debouncing - only return key if enough time has passed */
    if(key != 0) {
        if(key == last_key && (current_time - last_key_time) < JOYSTICK_DEBOUNCE_TIME) {
            /* Same key pressed too soon, ignore */
            return 0;
        }
        last_key = key;
        last_key_time = current_time;
        return key;
    }
    
    /* No key pressed, reset last key */
    last_key = 0;
    return 0;
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
 * Clear button states - should be called periodically
 */
void lv_port_indev_clear_buttons(void)
{
    btn_a_pressed = false;
    btn_b_pressed = false;
    btn_c_pressed = false;
    btn_d_pressed = false;
}

/**
 * Get the input group for adding UI objects
 * @return pointer to the input group
 */
lv_group_t * lv_port_indev_get_group(void)
{
    return input_group;
}