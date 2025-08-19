/*
 * FlowAssert.c
 *
 *  Created on: Aug 14, 2025
 *      Author: jez
 */

// FlowAssert.c
#include "FlowAssert.h"
#include "UARTDrv.h"     // para log por UART (o reemplazá por tu LOGGER_LOG)
#include "string.h"
#include <stdio.h>

#ifdef TEST_MODE

static const fa_step_t *s_plan = NULL;
static uint8_t s_plan_len = 0;
static uint8_t s_idx = 0;
static fa_status_t s_status = FA_IDLE;
static TickType_t s_step_started = 0;

static int stepHasEvent(const fa_step_t *st, event_id_t ev)
{
    for (uint8_t i = 0; i < st->count; i++) {
        if (st->alts[i] == ev) return 1;
    }
    return 0;
}

static void logStep(const char *prefix, uint8_t idx, event_id_t ev)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "%s step %u ev=%u\r\n", prefix, (unsigned)idx, (unsigned)ev);
    uartSendString(msg);
}

void flowAssertInit(const fa_step_t *plan, uint8_t plan_len)
{
    s_plan = plan;
    s_plan_len = plan_len;
    s_idx = 0;
    s_status = (plan && plan_len) ? FA_RUNNING : FA_FAILED;
    s_step_started = xTaskGetTickCount();
    uartSendString("[FlowAssert] BEGIN\r\n");
}

void flowAssertReset(void)
{
    s_idx = 0;
    s_status = (s_plan && s_plan_len) ? FA_RUNNING : FA_FAILED;
    s_step_started = xTaskGetTickCount();
    uartSendString("[FlowAssert] RESET\r\n");
}

fa_status_t flowAssertStatus(void)
{
    return s_status;
}

void flowAssertOnEvent(event_id_t ev)
{
    if (s_status != FA_RUNNING || !s_plan) return;

    const fa_step_t *st = &s_plan[s_idx];

    // Si el evento es uno esperado en este paso → avanzar
    if (stepHasEvent(st, ev)) {
        logStep("[FlowAssert] OK", s_idx, ev);
        s_idx++;
        if (s_idx >= s_plan_len) {
            s_status = FA_PASSED;
            uartSendString("[FlowAssert] PASSED\r\n");
            return;
        }
        s_step_started = xTaskGetTickCount();
        return;
    }

    // Si no es el esperado, ignoramos por defecto (hay mucho ruido de eventos en boot).
    // Si querés modo estricto, marcá FAIL al primer inesperado:
    // s_status = FA_FAILED;
    // log_step("[FlowAssert] UNEXPECTED", s_idx, ev);
}

void flowAssertPoll(void)
{
    if (s_status != FA_RUNNING || !s_plan) return;

    const fa_step_t *st = &s_plan[s_idx];
    TickType_t now = xTaskGetTickCount();
    if ((now - s_step_started) > st->timeout_ticks) {
        s_status = FA_FAILED;
        // Reportá cuál paso falló y qué alternativas esperaba
        uartSendString("[FlowAssert] TIMEOUT step ");
        char buf[32];
        snprintf(buf, sizeof(buf), "%u\r\n", (unsigned)s_idx);
        uartSendString(buf);
    }
}

#endif // TEST_MODE

