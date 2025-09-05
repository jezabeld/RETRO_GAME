/*
 * FlowPlans.c
 *
 *  Created on: Aug 14, 2025
 *      Author: jez
 */

#include "FlowAssert.h"
#include "synchronization.h"
#include "BootMng.h"

#ifdef TEST_MODE
// Paso 0: (dejarlo vacío y se salta)
static const event_id_t STEP0_OKS[] = { /* TFT_OK, AUD_OK, HAP_OK, MEM_OK, INP_OK, TIM_OK, DBG_OK */ };

// Paso 1..n - Updated for Beta 0.2
static const event_id_t STEP_FLAGREQ [] = { CFG_FLAG_REQ };
static const event_id_t STEP_BOOTCOMP[] = { SE_BOOT_COMPLETE };
static const event_id_t STEP_SHOWMENU[] = {SE_SHOW_MENU};
static const event_id_t STEP_GFXINIT[] = { SE_GFX_INIT }; // New: GraphEngine LVGL initialized
static const event_id_t STEP_HASSAVE[] = { CFG_HAS_SAVE, CFG_NO_SAVE };
// New: Test button inputs (A/B/C/D nomenclature)
static const event_id_t STEP_BUTTONS[] = { INP_BTN_A, INP_BTN_B, INP_BTN_C, INP_BTN_D };
//static const event_id_t STEP_JOYSTICK[] = { INP_JOY_UP, INP_JOY_DOWN, INP_JOY_LEFT, INP_JOY_RIGHT, INP_JOY_CENTER };

static const fa_step_t BOOT_PLAN[] = {
//    { STEP0_OKS,  sizeof(STEP0_OKS)/sizeof(STEP0_OKS[0]), pdMS_TO_TICKS(800) },   // opcional
    { STEP_FLAGREQ, 1, pdMS_TO_TICKS(300) },
    { STEP_BOOTCOMP,  1, pdMS_TO_TICKS(300)  },
	{ STEP_SHOWMENU, 1,  pdMS_TO_TICKS(300)  },
	{ STEP_GFXINIT, 1, pdMS_TO_TICKS(500) }, // Wait for LVGL initialization
    { STEP_HASSAVE, 2, pdMS_TO_TICKS(800) },
};

void flowPlanBootInit(void)
{
	flowAssertInit(BOOT_PLAN, sizeof(BOOT_PLAN)/sizeof(BOOT_PLAN[0]));
}

#endif
