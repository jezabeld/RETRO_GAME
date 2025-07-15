#include "lvgl.h"
#include "TFT_ST7735.h"      /*tu driver*/
#include "main.h"            /*para los handles generados por CubeMX*/
#include "API_uart.h"

extern SPI_HandleTypeDef hspi1;  /* creado por CubeMX */

/* instancia global del TFT */
extern tft_t myTft;

/* ---  flush callback  --------------------------------------------- */
static void my_flush_cb(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
	 LV_LOG_INFO("FLUSH %d,%d %d,%d",
	                area->x1, area->y1, area->x2, area->y2);
    /* El ST7735 usa coordenadas de 0-127 / 0-159 */
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    tftSetAddressWindow(&myTft, area->x1, area->y1, area->x2, area->y2);
    /* El buffer de LVGL está en formato RGB565 igual que el TFT */
//    HAL_SPI_Transmit(tft.hSpi, (uint8_t *)color_p, w * h * 2, HAL_MAX_DELAY);

    tftDrawImage(&myTft, area->x1, area->y1, w, h, (const uint16_t*)color_p);

    lv_disp_flush_ready(drv);        /*avisa a LVGL que terminó*/
}

/* ---  inicialización pública  ------------------------------------- */
void lv_port_disp_init(void)
{

    /* 2. Borra la GRAM: */
    tftFillScreen(&myTft, TFT_COLOR_GREEN);

    /* --- draw buffer de LVGL (una “línea” de 20 píxeles) */
    static lv_color_t buf[TFT_WIDTH * 10];
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * 10);

    /* --- registra el display driver */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    /* ①  FALTABA: decirle qué draw-buffer usar  */
    disp_drv.draw_buf = &draw_buf;

    /* ②  (opcional, pero más claro) --> la de arriba es la importante  */
    /* disp_drv.full_refresh = 0;   // modo por defecto                */

    disp_drv.hor_res  = TFT_WIDTH;        /* 128  */
    disp_drv.ver_res  = TFT_HEIGHT;       /* 160  */
    disp_drv.flush_cb = my_flush_cb;      /* callback válido   */
    disp_drv.user_data = &myTft;

    lv_disp_drv_register(&disp_drv);
}

void my_log_cb(const char * buf)
{
    /* envío directo por UART; usa tu API de consola */
    uartSendString(buf);
    uartSendString("\r\n");
}
