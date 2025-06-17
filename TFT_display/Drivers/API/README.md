# Drivers API

Este módulo contiene un conjunto de drivers para facilitar el uso de diversos periféricos en el microcontrolador STM32. Dichos drivers fueron desarrollados como parte de las prácticas de la materia *Programación de Microcontroladores*.

## Estructura del Proyecto

```
API/
  Inc/
    API_delay.h          # Proporciona funciones para retardos no bloqueantes
    API_debounce.h       # Implementa una máquina de estados para el debounce de un botón
    API_uart.h           # Interfaz para la configuración y uso de UART2
  Src/
    API_delay.c          # Implementación de las funciones de retardo no bloqueante
    API_debounce.c       # Implementación de la máquina de estados de debounce
    API_uart.c           # Implementación de las funciones de UART2
```

## API UART

El módulo `API_uart` facilita la inicialización y transmisión de datos a través del puerto UART2.

Funciones:
- `uartInit()`:Inicializa la UART2 con una configuración predeterminada (Baud rate: 115200, 8 bits de datos, sin paridad).
- `uartSendString()`: Envía un string completo a través de UART2. 
- `uartSendStringSize()`: Envía una cantidad específica de caracteres de un string a través de UART2.
- `uartReceiveStringSize()`: Recibe un string de longitud definida a través de UART2.
- `uartSendValue()`: Envía un valor entero de hasta 32 bits a través de UART2.

## API Delay

El módulo `API_delay` proporciona una implementación de retardos no bloqueantes utilizando un temporizador de sistema.

Funciones:
- `delayInit()`: Inicializa un retardo no bloqueante con la duración especificada.
- `delayRead()`: Lee y actualiza el estado del retardo. 
- `delayWrite()`: Modifica la duración de un retardo existente.
- `delayIsRunning()`: Consulta si un retardo está en ejecución.

## API Debounce

El módulo `API_debounce` implementa una máquina de estados finitos (FSM) para el debounce de un botón. Utiliza retardos no bloqueantes para evitar la detección errónea de pulsaciones.

Funciones:
- `debounceFSM_init()`: Inicializa la máquina de estados del debounce y configura el temporizador de debounce.
- `debounceFSM_update()`: Actualiza la máquina de estados del debounce y detecta cambios en el estado del botón (presionado, liberado, ruido).
- `readKey()`: Lee si el botón ha sido presionado desde la última actualización de la máquina de estados.

