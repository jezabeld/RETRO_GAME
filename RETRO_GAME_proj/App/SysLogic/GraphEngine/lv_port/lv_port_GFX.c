#include "lv_port_GFX.h"
#include "lv_port_indev.h"

static void fadeout_cb(void * obj, int32_t opa)
{
    lv_obj_set_style_opa(obj, opa, 0);   // aplicar opacidad al objeto
}

static void fadeout_ready_cb(lv_anim_t * a)
{
    lv_obj_t * obj = (lv_obj_t *)a->var;
    // opcional: ocultar o borrar objeto cuando termina
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    // o directamente lv_obj_del(obj) si no lo necesitás más
}

void ui_fadeout_screen(lv_obj_t * screen, uint32_t duration_ms)
{
    // Limpiar grupo de navegación antes del fadeout
    lv_group_t * input_group = lv_port_indev_get_group();
    if(input_group != NULL) {
        lv_group_remove_all_objs(input_group);
    }

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, screen);
    lv_anim_set_values(&a, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&a, duration_ms);
    lv_anim_set_exec_cb(&a, fadeout_cb);
    lv_anim_set_ready_cb(&a, fadeout_ready_cb);
    lv_anim_start(&a);
}
