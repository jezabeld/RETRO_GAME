# RETRO GAME PROJECT 

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
| PA4 | ADC1_IN4 | JY_VRY | 112 Cycles Sampling |

**DMA Configuration:**
- **DMA**: DMA2_Stream0, High Priority, Circular Mode
- **Mode**: Continuous Conversion, 2 Channels
- **Data Format**: 16-bit values, DMA Continuous Requests enabled

### GPIO - Digital Inputs (Buttons)
| Pin | Function | Configuration |
|-----|----------|---------------|
| PC0 | BTN_LEFT | External Interrupt (Falling Edge), Pull-up |
| PC1 | BTN_DOWN | External Interrupt (Falling Edge), Pull-up |
| PC2 | BTN_RIGHT | External Interrupt (Falling Edge), Pull-up |
| PC3 | BTN_UP | External Interrupt (Falling Edge), Pull-up |
| PC13 | Blue Button | External Interrupt (Falling Edge) |

**Interrupt Configuration:**
- All button interrupts configured with Priority 5
- FreeRTOS compatible interrupt priorities (5-15 range)

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
| DMA1 | Stream5 | USART2_RX | Periph→Memory | High |
| DMA1 | Stream6 | USART2_TX | Memory→Periph | High |
| DMA2 | Stream0 | ADC1 | Periph→Memory | High |
| DMA2 | Stream3 | SPI1_TX | Memory→Periph | High |

---

## Software Architecture

### FreeRTOS Configuration
- **Scheduler**: CMSIS-RTOS v1
- **Tick Rate**: 1000 Hz (1ms tick)
- **Priority Groups**: NVIC_PRIORITYGROUP_4
- **Heap Size**: 512 bytes (0x200)
- **Stack Size**: 1024 bytes (0x400)

### Software Components Status

#### ✅ **Implemented Components**
- **BootMng**: System initialization, driver setup, task creation, and initial event generation
- **EventDispatcher**: Central event router with routing table and trace functionality 
- **LogSink + CommandParser**: UART-based logging with command processing (logon/logoff)
- **UARTDrv**: Complete ring buffer implementation with DMA TX/RX and ISR-safe operations
- **Flow Testing Framework**: Automated event sequence validation for system testing (TEST_MODE)

#### 🚧 **Mock/Template Components** 
*Basic structure implemented, core logic pending*
- **SystemManager**: Task structure created, state machine logic placeholder
- **GameManager**: Task framework ready, game logic implementation pending  
- **RenderEngine**: Basic task, rendering pipeline not implemented
- **InputHandler**: Task created, input processing logic placeholder
- **UIController**: Task structure, user interface logic pending
- **GraphEngine**: Task with activity indicator, graphics operations pending
- **Persistence**: Task framework, save/load functionality placeholder
- **Hardware Drivers**: Basic init functions only (AccelDrv, AudioDrv, EEPROMDrv, ScreenDrv, HapticDrv, InputDrv, TimerDrv)

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

## Project Information
- **Project**: RETRO_GAME_proj
- **Current Version**: v0.1 (Beta)
- **Date**: August 2025
- **Board**: STM32 Nucleo-F446RE
- **IDE**: STM32CubeIDE 6.14.0

## Changes Added in v0.1 (Beta)
- **Initial hardware abstraction layer**: Complete driver implementation for all peripherals
- **Event-driven architecture**: Central EventDispatcher with FreeRTOS message queues
- **DMA-optimized UART**: Ring buffer implementation with non-blocking operations
- **Unified event system**: Single events.h file for all event definitions and queue declarations
- **Flow testing framework**: Automated event sequence validation system (TEST_MODE)
- **Multi-peripheral DMA**: Optimized stream allocation for UART, SPI, and ADC
- **Thread-safe operations**: FreeRTOS-compatible critical sections and ISR handling

**Status**: ⚠️ **Beta Version** - Hardware layer incomplete, core infrastructure complete, game logic implementation pending.

---

*This README will be updated with each new version release, documenting incremental changes and improvements.*