/*
 * syncrronization.c
 *
 *  Created on: Sep 11, 2025
 *      Author: jez
 */

#include "synchronization.h"

void sendEvent(uint8_t ev){
	event_id_t newEvent = ev;
	xQueueSend(qEvents, &newEvent, 0);
}

void sendEventQueue(uint8_t ev, QueueHandle_t queue){
	event_id_t newEvent = ev;
	xQueueSend(queue, &newEvent, 0);
}
