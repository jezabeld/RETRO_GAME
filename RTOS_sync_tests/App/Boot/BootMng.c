/**
 * @file BootMng.c
 * @brief Inicialización de tareas, colas, semáforos y eventos para FreeRTOS.
 *
 * Este módulo centraliza la configuración inicial del sistema.
 *
 * @date: Aug 12, 2025
 * @author: jez
 */

#include "BootMng.h"
#include "cmsis_os.h"

/* Incluye headers de cada componente */
#include "EventDispatcher.h"
#include "GraphEngine.h"
#include "GameManager.h"
#include "RenderEngine.h"
#include "InputHandler.h"
#include "UIController.h"
#include "SystemManager.h"
#include "Persistence.h"
#include "UARTDrv.h"
#include "AccelDrv.h"
#include "EEPROMDrv.h"
#include "ScreenDrv.h"
#include "AudioDrv.h"
#include "HapticDrv.h"
#include "InputDrv.h"
#include "TimerDrv.h"
#include "events.h"
#include "LogSink.h"
#include "CommandParser.h"
#ifdef TEST_MODE
#include "FlowPlans.h"
#endif

#define QUEUE_SIZE 10

/* ===== Handlers de tareas ===== */
TaskHandle_t tskEvntDisp;
TaskHandle_t tskGraph;
TaskHandle_t tskGameMng;
TaskHandle_t tskRender;
TaskHandle_t tskInpHnd;
TaskHandle_t tskUiCtrl;
TaskHandle_t tskSysMng;
TaskHandle_t tskPersist;
TaskHandle_t tskLogSink;

/* ===== Handlers de colas ===== */
QueueHandle_t qEvents;
QueueHandle_t qResLoad;
QueueHandle_t qPersist;
QueueHandle_t qUiCtrl;
QueueHandle_t qGraph;
QueueHandle_t qAudio;
QueueHandle_t qHaptic;
QueueHandle_t qLog;
QueueHandle_t qSystem;
QueueHandle_t qGame;
//QueueHandle_t qRender;
QueueHandle_t qInHand;

void bootInit(void)
{
	/* ===== INICIALIZAR DRIVERS ===== */
	uartInit(); // revisar requerimiento de que el trace debe encontrarse disponible desde el SPLASH
	uartSendString("BootMng: Iniciando sistema...\r\n");
	
	uartSendString("BootMng: Inicializando drivers...\r\n");
	accelDrvInit();
	eepromDrvInit();
	tftInit();
	audioInit();
	hapticInit();
	
	input_hw_cfg_t inputConfig = {.buttonCount = 4, .joystickChannels = 2};
	inputInit(&inputConfig);
	
	timerInitLvgl();
	uartSendString("BootMng: Drivers inicializados\r\n");

    BaseType_t ret;

    /* ===== CREACIÓN DE COLAS ===== */
    qEvents   = xQueueCreate(2*QUEUE_SIZE, sizeof(event_id_t)); configASSERT(qEvents);
    qResLoad  = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qResLoad);
    qPersist  = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qPersist);
    qUiCtrl   = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qUiCtrl);
    qGraph    = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qGraph);
    qAudio    = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qAudio);
    qHaptic   = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qHaptic);
    qLog      = xQueueCreate(2*QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qLog);
    qSystem   = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qSystem);
    qGame     = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qGame);
    //qRender   = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qRender);
    qInHand   = xQueueCreate(QUEUE_SIZE, sizeof(event_id_t));  configASSERT(qInHand);

    /* Registro para depuración */
    vQueueAddToRegistry(qEvents,  "qEvents");
    vQueueAddToRegistry(qSystem,  "qSystem");
    vQueueAddToRegistry(qPersist, "qPersist");
    vQueueAddToRegistry(qUiCtrl,  "qUiCtrl");
    vQueueAddToRegistry(qGraph,   "qGraph");
    vQueueAddToRegistry(qAudio,   "qAudio");
    vQueueAddToRegistry(qHaptic,  "qHaptic");
    vQueueAddToRegistry(qLog,     "qLog");
    vQueueAddToRegistry(qGame,    "qGame");
    vQueueAddToRegistry(qResLoad, "qResLoad");
    //vQueueAddToRegistry(qRender,  "qRender");
    vQueueAddToRegistry(qInHand,  "qInHand");

    /* ===== CREACIÓN DE TAREAS ===== */
    ret = xTaskCreate(EventDispatcherTask, "EvntDisp", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+5, &tskEvntDisp);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(GraphEngineTask, "Graph", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+4, &tskGraph);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(UIControllerTask, "UiCtrl", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &tskUiCtrl);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(SystemManagerTask, "SysMng", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &tskSysMng);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(PersistenceTask, "Persist", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &tskPersist);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(GameManagerTask, "GameMng", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &tskGameMng);
    configASSERT(pdPASS == ret);
    vTaskSuspend(tskGameMng);

    ret = xTaskCreate(RenderEngineTask, "Render", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &tskRender);
    configASSERT(pdPASS == ret);
    vTaskSuspend(tskRender);

    ret = xTaskCreate(InputHandlerTask, "InpHnd", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &tskInpHnd);
    configASSERT(pdPASS == ret);
    vTaskSuspend(tskInpHnd);

    ret = xTaskCreate(LogSinkTask, "LogSink", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &tskLogSink);
    configASSERT(pdPASS == ret);

    uartSendString("BootMng: inicializacion de colas y tareas completada\r\n");
    
    // Enviar eventos iniciales
    event_id_t bootEvent = CFG_FLAG_REQ;
    xQueueSend(qEvents, &bootEvent, 0);
    uartSendValue(bootEvent);
    uartSendString(" - BootMng: evento enviado\r\n");
    
    bootEvent = SE_BOOT_COMPLETE;
    xQueueSend(qEvents, &bootEvent, 0);
    uartSendValue(bootEvent);
    uartSendString(" - BootMng: evento enviado\r\n");

#ifdef TEST_MODE
    flowPlanBootInit();
#endif
}

