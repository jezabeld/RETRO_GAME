# Gráficos en TFT

## Opción 1: Librerías de mapas (glyphs / bitmaps)

Para dibujar letras, íconos, formas predefinidas:

- Usás matrices de bits (ej: fuente 5×8 píxeles, como en VGA o ASCII).
- Cada letra es un array de bytes, cada bit representa un píxel ON/OFF.
- Dibujás esos bits “a mano”, escribiendo los píxeles activos.

## Opción 2: Framebuffer en RAM (más RAM, más control)

- Creás un array RAM como `uint16_t framebuffer[128][160]`.
- Dibujás formas, texto, animaciones sobre el buffer en memoria.
- Cuando está listo, lo enviás a la pantalla con `RAMWR`.

Ventaja: podés hacer doble buffer, composiciones, animaciones sin flicker.
Desventaja: necesitás más RAM (~40 KB solo para 128×160 a 16 bits).

## Para cosas simples (letras, líneas, cuadros):
Sí, vas a necesitar tu propia librería o usar una existente que ya tenga funciones tipo:

- drawPixel(x, y, color)
- drawLine(x0, y0, x1, y1, color)
- drawChar(x, y, char, color, bg, size)
- drawBitmap(x, y, *data, w, h, color)

Estas funciones generan los píxeles correctos y los envían por SPI.


# STM32 ST7735 Display Driver (HAL + SPI)

Controlador básico para pantallas ST7735 en STM32 usando HAL y comunicación SPI en modo half-duplex. Este proyecto permite inicializar el display, configurar la ventana de dibujo y pintar píxeles o figuras simples directamente mediante comandos SPI.

## Requisitos

- STM32 (desarrollado y probado en NUCLEO-F446RE)
- STM32CubeIDE
- Comunicación SPI con los siguientes pines:
  - MOSI (half-duplex, no se usa MISO)
  - CS (Chip Select)
  - DC (Data/Command)
  - RESET (opcional, pero recomendado)
- Biblioteca STM32 HAL habilitada

## Conexiones recomendadas

| Señal | Descripción        | MCU Pin Ejemplo  |
|-------|--------------------|------------------|
| CS    | Chip Select        | `PB6`            |
| DC    | Data/Command       | `PA6`            |
| RESET | Reset del display  | `PC7`            |
| SDA   | SPI MOSI           | `PA7`            |
| SCK   | SPI CLK            | `PA5`            |
| BKL   | 3.3V               | -                |
| VCC   | 5.0V               | -                |
| GND   | GND                | -                |


## Estructura del driver

### Estructura principal `tft_t`

```c
typedef struct {
    SPI_HandleTypeDef *hSpi;
    GPIO_TypeDef *csPort;
    uint16_t csPin;
    GPIO_TypeDef *dcPort;
    uint16_t dcPin;
    GPIO_TypeDef *resPort;
    uint16_t resPin;
} tft_t;
```

#### Inicializacion
```c
void tftInit(tft_t *tft, SPI_HandleTypeDef *hSpi,
             GPIO_TypeDef *csPort, uint16_t csPin,
             GPIO_TypeDef *dcPort, uint16_t dcPin,
             GPIO_TypeDef *resPort, uint16_t resPin);
```
Esta función configura los pines, inicializa el SPI y envía la secuencia de comandos de inicialización (compatible con pantallas ST7735R tipo "red tab").

#### Comandos
```c
void sendCommand(tft_t *tft, uint8_t command, const uint8_t *data, uint8_t len);
```
Envía un comando al display con sus parámetros. Se usa internamente por otras funciones.

#### Dirección de ventana
```c
void tftSetAddressWindow(tft_t *tft, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
```
Establece una región rectangular de la pantalla donde se enviarán los siguientes datos de píxeles.

#### Pintar la pantalla
```c
void tftFillScreen(tft_t *tft, uint16_t color);
```
Rellena la pantalla completa con un color RGB565.

#### Dibujar píxeles y rectángulos
```c
void tftDrawPixel(tft_t *tft, uint16_t x, uint16_t y, uint16_t color);
void tftFillRectangle(tft_t * tft, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
```
Permiten escribir un solo píxel.

#### Escribir caracteres y textos
```c
void tftWriteChar(tft_t * tft, uint8_t x, uint8_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void tftWriteString(tft_t * tft, uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
```

#### Formato de color
Todos los colores se expresan en formato RGB565 (16 bits). Ejemplos de colores pre-codificados:
```c
#define	TFT_COLOR_BLACK   0x0000
#define	TFT_COLOR_BLUE    0x001F
#define	TFT_COLOR_RED     0xF800
#define	TFT_COLOR_GREEN   0x07E0
#define TFT_COLOR_CYAN    0x07FF
#define TFT_COLOR_MAGENTA 0xF81F
#define TFT_COLOR_YELLOW  0xFFE0
#define TFT_COLOR_WHITE   0xFFFF
```
