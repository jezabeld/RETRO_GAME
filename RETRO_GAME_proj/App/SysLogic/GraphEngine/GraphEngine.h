/*
 * GraphEngine.h
 *
 *  Created on: Aug 12, 2025
 *      Author: jez
 */

#ifndef SYSLOGIC_GRAPHENGINE_GRAPHENGINE_H_
#define SYSLOGIC_GRAPHENGINE_GRAPHENGINE_H_

#include "cmsis_os.h"

/* Bits de notificación (canal 0) */
#define GFXN_UI   (1u << 0)
#define GFXN_GAME (1u << 1)

void GraphEngineTask(void *pvParameters);

#endif /* SYSLOGIC_GRAPHENGINE_GRAPHENGINE_H_ */
