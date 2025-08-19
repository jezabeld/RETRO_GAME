/*
 * FlowAssert.h
 *
 *  Created on: Aug 14, 2025
 *      Author: jez
 */

#ifndef INTEGRATION_FLOW_FLOWASSERT_H_
#define INTEGRATION_FLOW_FLOWASSERT_H_

#include <stdint.h>
#include "cmsis_os.h"
#include "events.h"
#include "BootMng.h" // define TEST_MODE

#ifdef TEST_MODE   // activar sólo en builds de prueba

typedef struct {
    const event_id_t *alts;   // puntero a arreglo de alternativas válidas
    uint8_t           count;  // cantidad de alternativas
    TickType_t        timeout_ticks; // timeout máximo para alcanzar este paso (relativo al arranque del paso)
} fa_step_t;

typedef enum {
    FA_IDLE = 0,
    FA_RUNNING,
    FA_PASSED,
    FA_FAILED
} fa_status_t;

void flowAssertInit(const fa_step_t *plan, uint8_t plan_len);
void flowAssertReset(void);
fa_status_t flowAssertStatus(void);

// Llamar cuando el dispatcher vea *cualquier* evento en el sistema
void flowAssertOnEvent(event_id_t ev);

// Llamar periódicamente desde una tarea existente (p.ej. LogSinkTask)
void flowAssertPoll(void);

#else
// Compilación vacía
#define flowAssertInit(p,l)      ((void)0)
#define flowAssertReset()         ((void)0)
#define flowAssertStatus()        (0)
#define flowAssertOnEvent(e)      ((void)0)
#define flowAssertPoll()          ((void)0)
#endif


#endif /* INTEGRATION_FLOW_FLOWASSERT_H_ */
