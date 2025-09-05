# RETRO GAME PROJECT 

**Status**: ⚠️ **Beta Version** - Hardware layer incomplete, core infrastructure incomplete, game logic pending.

## Project Information
- **Project**: RETRO_GAME_proj
- **Current Version**: v0.2 (Beta)
- **Date**: September 2025
- **Board**: STM32 Nucleo-F446RE
- **IDE**: STM32CubeIDE 6.14.0

## Hardware Configuration - STM32F446RE (Nucleo Board)

### System Clock Configuration
- **Main Clock**: 84 MHz (HSE 8MHz + PLL)
- **APB1 Clock**: 42 MHz (HCLK/2) 
- **APB2 Clock**: 84 MHz
- **Flash Latency**: 2 Wait States

---

## Peripheral Pin Assignment

### UART2 - Debug/Communication Interface
| Pin | Function | Configuration | DMA |
|-----|----------|---------------|-----|
| PA2 | TX | Async Mode, 115200 baud | DMA1_Stream6 |
| PA3 | RX | Async Mode, 115200 baud | DMA1_Stream5 (Circular) |

**DMA Configuration:**
- **TX DMA**: DMA1_Stream6, High Priority, Normal Mode
- **RX DMA**: DMA1_Stream5, High Priority, Circular Mode with ReceiveToIdle
- **Ring Buffer**: 512 bytes TX, 128 bytes RX
- **Implementation**: Non-blocking UART driver with automatic DMA management

### SPI1 - TFT Display Interface  
| Pin | Function | Configuration |
|-----|----------|---------------|
| PA5 | SCK | Master Mode, 42 MBits/s |
| PA7 | MOSI | Simplex Bidirectional |
| PA6 | TFT_DC | GPIO Output (Data/Command) |
| PB6 | TFT_CS | GPIO Output (Chip Select, Active Low) |
| PC7 | TFT_RS | GPIO Output (Reset, Active Low) |

**DMA Configuration:**
- **TX DMA**: DMA2_Stream3, High Priority, Normal Mode, Memory to Peripheral
- **Speed**: 42 MBits/s (APB2 Clock / 2)

### ADC1 - Joystick Analog Input
| Pin | Channel | Function | Configuration |
|-----|---------|----------|---------------|
| PA1 | ADC1_IN1 | JY_VRX | 112 Cycles Sampling |
| PA0 | ADC1_IN0 | JY_VRY | 112 Cycles Sampling |

**DMA Configuration:**
- **DMA**: DMA2_Stream0, High Priority, Circular Mode
- **Mode**: Continuous Conversion, 2 Channels
- **Data Format**: 16-bit values, DMA Continuous Requests enabled

### GPIO - Digital Inputs (Buttons)
| Pin | Function | Configuration |
|-----|----------|---------------|
| PC0 | BTN_A | External Interrupt (Falling Edge), Pull-up |
| PC1 | BTN_B | External Interrupt (Falling Edge), Pull-up |
| PC2 | BTN_C | External Interrupt (Falling Edge), Pull-up |
| PC3 | BTN_D | External Interrupt (Falling Edge), Pull-up |
| PC13 | Blue Button | External Interrupt (Falling Edge) |

**Interrupt Configuration:**
- All button interrupts configured with Priority 5
- FreeRTOS compatible interrupt priorities (5-15 range)
- FreeRTOS software timers for debounce implementation

### TIM3/TIM6 - PWM Audio Output
| Pin | Timer | Channel | Function | Configuration |
|-----|-------|---------|----------|---------------|
| PB0 | TIM3 | CH3 | Audio Output | PWM Generation, 840 levels resolution |

**DMA Configuration:**
- **Audio DMA**: DMA1_Stream1, High Priority, Circular Mode
- **Trigger**: TIM6_UP (Timer 6 Update Event)
- **Sample Rate**: 10 kHz (AUDIO_FS_HZ)
- **Resolution**: 840 PWM levels (0-839 range)
- **Buffer**: Circular DMA buffer for continuous audio streaming

**Timer Configuration:**
- **TIM3**: PWM generation with variable duty cycle for audio output
- **TIM6**: Sample rate timer (10 kHz) triggering DMA transfers for audio buffer

### Debug Interface
| Pin | Function |
|-----|----------|
| PA13 | SWDIO (SWD Data I/O) |
| PA14 | SWCLK (SWD Clock) |
| PB3 | SWO (Serial Wire Output) |

---

## DMA Stream Allocation Summary

| DMA Controller | Stream | Peripheral | Direction | Priority |
|----------------|--------|------------|-----------|----------|
| DMA1 | Stream1 | TIM6_UP (Audio) | Memory→Periph | Low |
| DMA1 | Stream5 | USART2_RX | Periph→Memory | Low |
| DMA1 | Stream6 | USART2_TX | Memory→Periph | Medium |
| DMA2 | Stream0 | ADC1 | Periph→Memory | High |
| DMA2 | Stream3 | SPI1_TX | Memory→Periph | High |

---

## Software Architecture

### FreeRTOS Configuration
- **Scheduler**: CMSIS-RTOS v1
- **Tick Rate**: 1000 Hz (1ms tick)
- **Priority Groups**: NVIC_PRIORITYGROUP_4
- **Heap Size**: 25KB (configTOTAL_HEAP_SIZE)
- **Tick Hook**: Enabled (configUSE_TICK_HOOK = 1)
- **Software Timers**: Enabled with daemon task priority 6

### Software Components Status

#### ✅ **Implemented Components**
- **BootMng**: System initialization, driver setup, task creation, and initial event generation
- **EventDispatcher**: Central event router with routing table and trace functionality 
- **LogSink + CommandParser**: UART-based logging with command processing (logon/logoff)
- **UARTDrv**: Complete ring buffer implementation with DMA TX/RX and ISR-safe operations
- **GraphEngine**: LVGL graphics rendering with DMA-accelerated display driver
- **UIController**: UI with button and joystick input processing
- **AudioDrv**: PWM-based audio driver with DMA circular buffer streaming
- **AudioPlayer**: High-level audio system with sound effects and waveform generation
- **AudioPlayerTask**: FreeRTOS task for audio event processing with beep sound library
- **InputDrv**: GPIO interrupt-based input system with FreeRTOS timer integration
- **ScreenDrv**: ST7735 TFT display driver 
- **Flow Testing Framework**: Automated event sequence validation for system testing (TEST_MODE)

#### 🚧 **Template/Placeholder Components** 
*Framework ready, core logic pending*
- **SystemManager**: State machine framework, minimal logic implemented for startup.
- **GameManager**: Task framework ready, game logic implementation pending  
- **RenderEngine**: Basic task, rendering pipeline not implemented
- **InputHandler**: Task created, input processing logic placeholder
- **Persistence**: Task framework, save/load functionality placeholder
- **Hardware Drivers**: Template drivers (AccelDrv, EEPROMDrv, HapticDrv, TimerDrv) with basic init functions

#### 📋 **Architecture Features**
- **Event-Driven Design**: Complete message queue system with centralized routing
- **FreeRTOS Integration**: Task creation, queue management, and thread-safe operations
- **DMA Optimization**: Non-blocking peripheral operations for UART, SPI, and ADC
- **Modular Structure**: Clean separation between hardware abstraction, system logic, and game logic

### Build Configuration  
- **Target**: STM32F446RETx
- **Toolchain**: STM32CubeIDE (GCC ARM)
- **HAL Version**: STM32Cube FW_F4 V1.28.1
- **Debug**: SWD Interface, 2 Wait States Flash

---

## Changes Added in v0.2 (Beta)

### TFT Display & LVGL Graphics Library Integration
- **ScreenDrv timing fix**: Custom `busyDelay()` function using CPU cycle counting to replace HAL_Delay() before scheduler start
- **DMA-accelerated rendering**: SPI1 with DMA2_Stream3 for high-speed display updates
- **Display driver integration**: Complete ST7735 driver with LVGL compatibility layer
- **Complete UI system**: Full LVGL v8.x implementation with ST7735 TFT display (128x160 RGB565)
- **Task synchronization**: Binary semaphores for GraphEngine ↔ UIController coordination with two-phase initialization
- **Memory integration**: Custom FreeRTOS memory management with realloc implementation
- **RTOS porting layer**: Custom `lv_port_rtos.c/h` files for FreeRTOS tick hook integration (Critical config: `configUSE_TICK_HOOK 1` enables proper LVGL timer processing and input event handling)

### Button Input System Implementation  
- **Hardware setup**: 4 buttons (A/B/C/D) with GPIO EXTI interrupts on PC0-PC3 pins
- **FreeRTOS timer integration**: Software timers created for future debounce implementation

### Audio System Implementation
- **Full PWM-based audio system**: TIM3_CH3 PWM output on PB0 with TIM6_UP DMA circular buffer streaming
- **Multi-layer architecture**: AudioDrv (low-level PWM), AudioPlayer (high-level effects), AudioPlayerTask (FreeRTOS event handling)
- **Sound effects library**: 4 different beep sounds with multiple waveforms (SINE, SQUARE, TRIANGLE, SAW, NOISE)
- **Event-driven audio feedback**: Events received by the Audio Player automatically trigger corresponding audio feedback 

### FreeRTOS Memory Optimization
- **Heap expansion**: Increased from 15KB to 25KB (`configTOTAL_HEAP_SIZE`) for LVGL requirements  
- **Task stack optimization**: GraphEngine (6x stack), UIController (4x stack) for rendering and UI management
- **Stack monitoring**: `uxTaskGetStackHighWaterMark()` implementation for runtime memory analysis (needs enabling in `FreeRTOSConfig.h`)
- **Memory stability**: Resolves heap corruption and stack overflow issues within 128KB RAM constraint


## Changes Added in v0.1 (Beta)
- **Initial hardware abstraction layer**: Complete driver implementation for all peripherals
- **Event-driven architecture**: Central EventDispatcher with FreeRTOS message queues
- **DMA-optimized UART**: Ring buffer implementation with non-blocking operations
- **Unified event system**: Single events.h file for all event definitions and queue declarations
- **Flow testing framework**: Automated event sequence validation system (TEST_MODE)
- **Multi-peripheral DMA**: Optimized stream allocation for UART, SPI, and ADC
- **Thread-safe operations**: FreeRTOS-compatible critical sections and ISR handling

---

*This README will be updated with each new version release, documenting incremental changes and improvements.*