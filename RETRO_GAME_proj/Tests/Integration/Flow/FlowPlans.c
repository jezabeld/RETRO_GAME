/*
 * FlowPlans.c
 *
 *  Created on: Aug 14, 2025
 *      Author: jez
 */

#include "FlowAssert.h"
#include "synchronization.h"

#ifdef TEST_MODE
// Paso 0: (dejarlo vacío y se salta)
static const event_id_t STEP0_OKS[] = { /* TFT_OK, AUD_OK, HAP_OK, MEM_OK, INP_OK, TIM_OK, DBG_OK */ };

// Paso 1..n
static const event_id_t STEP_FLAGREQ [] = { CFG_FLAG_REQ };
static const event_id_t STEP_BOOTCOMP[] = { SE_BOOT_COMPLETE };
static const event_id_t STEP_SHOWMENU[] = {SE_SHOW_MENU};
static const event_id_t STEP_HASSAVE[] = { CFG_HAS_SAVE, CFG_NO_SAVE };
// Si tenés eventos propios para “UI/menú listo”, agregalos acá
// static const event_id_t STEP4_MENU[] = { UI_MENU_READY }; // opcional

static const fa_step_t BOOT_PLAN[] = {
//    { STEP0_OKS,  sizeof(STEP0_OKS)/sizeof(STEP0_OKS[0]), pdMS_TO_TICKS(800) },   // opcional
    { STEP_FLAGREQ, 1, pdMS_TO_TICKS(300) },
    { STEP_BOOTCOMP,  1, pdMS_TO_TICKS(300)  },
	{STEP_SHOWMENU, 1,  pdMS_TO_TICKS(300)  },
    { STEP_HASSAVE, 2, pdMS_TO_TICKS(800) },
    // { STEP4_MENU, 1, TO(500) },
};

void flowPlanBootInit(void)
{
	flowAssertInit(BOOT_PLAN, sizeof(BOOT_PLAN)/sizeof(BOOT_PLAN[0]));
}

#endif
