/**
 * @file lv_port_disp.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include "TFT_ST7735.h"      /*tu driver*/
#include "main.h"            /*para los handles generados por CubeMX*/
#include "API_uart.h"

/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES    TFT_WIDTH
#define MY_DISP_VER_RES    TFT_HEIGHT

/**********************
 *      EXTERNAL DATA
 **********************/
extern SPI_HandleTypeDef hspi1;  /* creado por CubeMX */
/* instancia global del TFT */
extern tft_t myTft;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);
static void disp_flush_DMA(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_drv_t disp_drv;                      /*Descriptor of a display driver (global para usar con DMA)*/
static lv_disp_t * disp_drv_ready = NULL; 			/* usado como flag de inicialización    */

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/* ---  inicialización pública  ------------------------------------- */
void lv_port_disp_init(void)
{
	/*-------------------------
	 * Initialize your display
	 * -----------------------*/
	tftInit(&myTft, &hspi1,
			TFT_CS_GPIO_Port, TFT_CS_Pin,
			TFT_DC_GPIO_Port, TFT_DC_Pin,
			TFT_RS_GPIO_Port, TFT_RS_Pin);

	 /**
	 * LVGL requires a buffer where it internally draws the widgets.
	 * Later this buffer will passed to your display driver's `flush_cb` to copy its content to your display.
	 * The buffer has to be greater than 1 display row
	 *
	 * There are 3 buffering configurations:
	 * 1. Create ONE buffer:
	 *      LVGL will draw the display's content here and writes it to your display
	 *
	 * 2. Create TWO buffer:
	 *      LVGL will draw the display's content to a buffer and writes it your display.
	 *      You should use DMA to write the buffer's content to the display.
	 *      It will enable LVGL to draw the next part of the screen to the other buffer while
	 *      the data is being sent form the first buffer. It makes rendering and flushing parallel.
	 *
	 * 3. Double buffering
	 *      Set 2 screens sized buffers and set disp_drv.full_refresh = 1.
	 *      This way LVGL will always provide the whole rendered screen in `flush_cb`
	 *      and you only need to change the frame buffer's address.
	 */

    /* Example for 1): draw buffer de LVGL (una “línea” de 20 píxeles) */
//	static lv_disp_draw_buf_t draw_buf;
//    static lv_color_t buf[MY_DISP_HOR_RES * 10];                          /*A buffer for 10 rows*/
//    lv_disp_draw_buf_init(&draw_buf, buf, NULL, MY_DISP_HOR_RES * 10);   /*Initialize the display buffer*/

    /* Example for 2) DMA*/
    static lv_disp_draw_buf_t draw_buf_dsc_2;
    static lv_color_t buf_2_1[MY_DISP_HOR_RES * 40];                        /*A buffer for 40 rows*/
    static lv_color_t buf_2_2[MY_DISP_HOR_RES * 40];                        /*An other buffer for 40 rows*/
    lv_disp_draw_buf_init(&draw_buf_dsc_2, buf_2_1, buf_2_2, MY_DISP_HOR_RES * 40);   /*Initialize the display buffer*/

    /* Example for 3) also set disp_drv.full_refresh = 1 below*/
//    static lv_disp_draw_buf_t draw_buf_dsc_3;
//    static lv_color_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*A screen sized buffer*/
//    static lv_color_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES];            /*Another screen sized buffer*/
//    lv_disp_draw_buf_init(&draw_buf_dsc_3, buf_3_1, buf_3_2, MY_DISP_VER_RES * LV_VER_RES_MAX);   /*Initialize the display buffer*/

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/
//    static lv_disp_drv_t disp_drv;                      /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                   		/*Basic initialization*/

    /*Set up the functions to access to your display*/
    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;        /* 128  */
    disp_drv.ver_res = MY_DISP_VER_RES;       /* 160  */

    /*Used to copy the buffer's content to the display*/
//    disp_drv.flush_cb = disp_flush;
    disp_drv.flush_cb = disp_flush_DMA;

    /*Set a display buffer*/
//    disp_drv.draw_buf = &draw_buf; // para ejemplo 1
    disp_drv.draw_buf = &draw_buf_dsc_2;

    /* opcional */
     disp_drv.full_refresh = 0;   // modo por defecto PARTIAL
    //disp_drv.direct_mode = 1;
    /*Required for Example 3)*/
	//disp_drv.full_refresh = 1;

     /* Fill a memory array with a color if you have GPU.
	 * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
	 * But if you have a different GPU you can use with this callback.*/
	//disp_drv.gpu_fill_cb = gpu_fill;

    disp_drv.user_data = &myTft;

    /*Finally register the driver*/
    disp_drv_ready = lv_disp_drv_register(&disp_drv);	/* y setea el flag de inicializacion del driver  */
}

void my_log_cb(const char * buf)
{
    /* envío directo por UART; usa tu API de consola */
    uartSendString(buf);
    uartSendString("\r\n");
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	if(hspi == myTft.hSpi){
		if(disp_drv_ready) lv_disp_flush_ready(&disp_drv);
		tftUnselect(&myTft);
	}
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
	 LV_LOG_INFO("FLUSH %d,%d %d,%d", area->x1, area->y1, area->x2, area->y2);
    /* El ST7735 usa coordenadas de 0-127 / 0-159 */
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    tftSetAddressWindow(&myTft, area->x1, area->y1, area->x2, area->y2);
    /* El buffer de LVGL está en formato RGB565 igual que el TFT */
    tftDrawImage(&myTft, area->x1, area->y1, w, h, (const uint16_t*)color_p);

    /*IMPORTANT!!!
         *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(drv);        /*avisa a LVGL que terminó*/
}

static void disp_flush_DMA(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
	 LV_LOG_INFO("FLUSH %d,%d %d,%d", area->x1, area->y1, area->x2, area->y2);
    /* El ST7735 usa coordenadas de 0-127 / 0-159 */
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    tftSetAddressWindow(&myTft, area->x1, area->y1, area->x2, area->y2);
    /* El buffer de LVGL está en formato RGB565 igual que el TFT */
    tftDrawImageDMA(&myTft, area->x1, area->y1, w, h, (const uint16_t*)color_p);

    /*IMPORTANT!!!
         *Inform the graphics library that you are ready with the flushing*/
//    lv_disp_flush_ready(drv);        /*esto lo hace el callback de DMA*/
}

