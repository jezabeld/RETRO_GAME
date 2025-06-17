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