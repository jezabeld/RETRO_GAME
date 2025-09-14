/*
 * gameScreen.c
 *
 *  Created on: Sep 11, 2025
 *      Author: jez
 */

#include "gameScreen.h"

/* --- Recursos (en FLASH) --------------------------------------------- */
/* resources that live in FLASH (const) */
LV_IMG_DECLARE(cockpit128x160transp); /* generated PNG */

/* --- Estáticos de la pantalla ---------------------------------------- */
static lv_obj_t* scr_game      = NULL;
static lv_obj_t* cont_horizon  = NULL;   /* cielo */
static lv_obj_t* rect_ground   = NULL;   /* franja suelo */
static lv_obj_t* img_cockpit   = NULL;   /* overlay con alpha */

static lv_color_t SKY  = LV_COLOR_MAKE(0x74,0x9c,0xdb);
static lv_color_t DIRT = LV_COLOR_MAKE(0x46,0x33,0x21);

static inline uint16_t hor_res(void){ return lv_disp_get_hor_res(NULL); }
static inline uint16_t ver_res(void){ return lv_disp_get_ver_res(NULL); }

/* Si tu BSP expone el grupo de navegación, usalo; si no, podés usar el default. */
//extern lv_group_t* lv_port_indev_get_group(void); /* si no existe, comentá esto */

/* --- Builder ---------------------------------------------------------- */
void game_screen_create(void)
{
    if (scr_game) return; /* ya creada */

    const uint16_t W = hor_res();
    const uint16_t H = ver_res();

    /* Screen raíz */
    scr_game = lv_obj_create(NULL);
    lv_obj_clear_flag(scr_game, LV_OBJ_FLAG_SCROLLABLE);

    /* Capa 0: horizonte (cielo liso o con bg_img si querés) */
    cont_horizon = lv_obj_create(scr_game);
    lv_obj_remove_style_all(cont_horizon);
    lv_obj_set_size(cont_horizon, W, H);
    lv_obj_set_style_bg_opa(cont_horizon, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(cont_horizon, SKY, 0);
    lv_obj_clear_flag(cont_horizon, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* Capa 1: suelo (altura variable) */
    rect_ground = lv_obj_create(cont_horizon);
    lv_obj_remove_style_all(rect_ground);
    lv_obj_set_width(rect_ground, W);
    lv_obj_set_align(rect_ground, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(rect_ground, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(rect_ground, DIRT, 0);
    lv_obj_set_height(rect_ground, H / 3);
    lv_obj_clear_flag(rect_ground, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    /* Capa 2: cockpit (overlay con alpha) */
    img_cockpit = lv_img_create(scr_game);
    lv_img_set_src(img_cockpit, &cockpit128x160transp);
    lv_obj_align(img_cockpit, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_clear_flag(img_cockpit, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_move_foreground(img_cockpit); /* garantizar overlay */

    /* Limpiar el grupo de navegación para evitar que queden focos “viejos” */
    lv_group_t* g = NULL;
    #ifdef lv_port_indev_get_group
      g = lv_port_indev_get_group();
    #endif
    if(!g) g = lv_group_get_default();
    if (g) lv_group_remove_all_objs(g);
}

/* --- Mostrar/ocultar -------------------------------------------------- */
void game_screen_show(bool fade_in)
{
    if (!scr_game) game_screen_create();

    if (fade_in) {
        /* Fade desde la pantalla actual hacia la del juego */
        lv_scr_load_anim(scr_game, LV_SCR_LOAD_ANIM_FADE_IN, 250, 0, false);
    } else {
        lv_scr_load(scr_game);
    }
}

/* --- Destruir (si cambiás de modo y querés liberar) ------------------- */
void game_screen_destroy(void)
{
    if (!scr_game) return;
    /* Si esta screen es activa, conviene cargar una “vacía” antes */
    if (lv_scr_act() == scr_game) {
        lv_obj_t* blank = lv_obj_create(NULL);
        lv_obj_remove_style_all(blank);
        lv_obj_set_size(blank, hor_res(), ver_res());
        lv_obj_set_style_bg_opa(blank, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(blank, lv_color_black(), 0);
        lv_scr_load(blank);
    }
    lv_obj_del(scr_game);
    scr_game = cont_horizon = rect_ground = img_cockpit = NULL;
}

/* --- Updates en runtime ---------------------------------------------- */
void game_screen_set_ground_height_px(uint16_t h_px)
{
    if (!rect_ground) return;
    uint16_t H = ver_res();
    if (h_px > H) h_px = H;
    /* Ajusta bottom-rect (alto = cuánto suelo “invade” la pantalla) */
    lv_obj_set_height(rect_ground, h_px);
}

/* Helper para mapear tu ADC (0..4095) a 0..ver_res (h suelo) */
void game_screen_set_pitch_raw(uint16_t adc0_4095)
{
    uint16_t H = ver_res();
    if (adc0_4095 > 4095) adc0_4095 = 4095;
    /* lv_map está en LVGL, pero lo replicamos para evitar depender de inline:
       0  -> H  (picado fuerte = mucho suelo)
       4095 -> 0 (trepada = poco suelo) */
    uint32_t num = (uint32_t)(4095 - adc0_4095) * H;
    uint16_t h = (uint16_t)(num / 4095u);
    game_screen_set_ground_height_px(h);
}

