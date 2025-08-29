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
#include "main.h"

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
#include "synchronization.h"
#include "LogSink.h"
#include "CommandParser.h"
#include "images.h"
#ifdef TEST_MODE
#include "FlowPlans.h"
#endif

#define QUEUE_SIZE 10
#define AUDIO_FS_HZ   10000u   // frecuencia de muestreo (TIM6 Update)

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

/* ===== Handlers de semáforos de sincronización ===== */
SemaphoreHandle_t semGraphEngineReady;
SemaphoreHandle_t semUiReady;

tft_t myTft;
extern SPI_HandleTypeDef hspi1;

void bootInit(void)
{
	/* ===== INICIALIZAR DRIVERS ===== */
	uartInit(); // revisar requerimiento de que el trace debe encontrarse disponible desde el SPLASH
	uartSendString("BootMng: Iniciando sistema...\r\n");
	
	uartSendString("BootMng: Inicializando drivers...\r\n");
	accelDrvInit();
	eepromDrvInit();
	tftInit(&myTft, &hspi1, TFT_CS_GPIO_Port, TFT_CS_Pin, TFT_DC_GPIO_Port, TFT_DC_Pin, TFT_RS_GPIO_Port, TFT_RS_Pin);
	tftFillScreen(&myTft, 0x0000); // Limpiar pantalla con negro
	tftDrawImage(&myTft, 0, 0, TFT_WIDTH, TFT_HEIGHT, (uint16_t*)imgSplash);
	audioInit(AUDIO_FS_HZ);
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

    /* ===== CREACIÓN DE SEMÁFOROS ===== */
    semGraphEngineReady = xSemaphoreCreateBinary();
    semUiReady = xSemaphoreCreateBinary();
    configASSERT(semGraphEngineReady && semUiReady);

    /* Registro para depuración */
    vQueueAddToRegistry(semGraphEngineReady, "Semáforo GFX");
    vQueueAddToRegistry(semUiReady, "Semáforo UI");

    /* ===== CREACIÓN DE TAREAS ===== */
    ret = xTaskCreate(EventDispatcherTask, "EvntDisp", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+5, &tskEvntDisp);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(GraphEngineTask, "Graph", 6*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+4, &tskGraph);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(UIControllerTask, "UiCtrl", 4*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &tskUiCtrl);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(SystemManagerTask, "SysMng", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+2, &tskSysMng);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(PersistenceTask, "Persist", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &tskPersist);
    configASSERT(pdPASS == ret);

    ret = xTaskCreate(GameManagerTask, "GameMng", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &tskGameMng);
    configASSERT(pdPASS == ret);
    vTaskSuspend(tskGameMng);

    ret = xTaskCreate(RenderEngineTask, "Render", 2*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &tskRender);
    configASSERT(pdPASS == ret);
    vTaskSuspend(tskRender);

    ret = xTaskCreate(InputHandlerTask, "InpHnd", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+3, &tskInpHnd);
    configASSERT(pdPASS == ret);
    vTaskSuspend(tskInpHnd);

    ret = xTaskCreate(LogSinkTask, "LogSink", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY+1, &tskLogSink);
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

