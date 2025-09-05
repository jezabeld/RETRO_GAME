# CHANGELOG - RETRO_GAME_feat_RTOS

## [Beta 0.2] - 2025-08-29

### Major System Integration Achievements

#### ✅ Audio System Migration (DAC → PWM)
- **Problem**: DMA stream conflicts between DAC audio and existing UART2_RX (both using DMA1_Stream5)
- **Solution**: Migrated from DAC to PWM-based audio generation
  - Reconfigured hardware: PA4 freed, moved audio output to PB0
  - Implemented TIM3_CH3 PWM output with TIM6_UP DMA using DMA1_Stream1
  - Updated `RETRO_GAME_proj.ioc` with new timer configurations
- **Files Modified**: `AudioDrv.c/h`, `RETRO_GAME_proj.ioc`, `main.c/h`

#### ✅ Function Naming Standardization
- **Problem**: Inconsistent naming conventions across audio driver functions
- **Solution**: Converted all audio functions to camelCase without underscores
  - `AudioPwmDrv_*` → `audio*`
  - `audio_dma_half_callback` → `audioDmaHalfCallback`
  - `audio_dma_complete_callback` → `audioDmaFullCallback`
  - `map12_to_duty` → `mapToDuty`
- **Files Modified**: `AudioDrv.c/h`, `BootMng.c`

#### ✅ Button Input System Implementation
- **Hardware Setup**: Updated button nomenclature and GPIO configuration
  - Changed from directional names to generic: BTN_UP → BTN_D, BTN_DOWN → BTN_A, BTN_LEFT → BTN_B, BTN_RIGHT → BTN_C
  - Configured GPIO EXTI interrupts for 4 buttons (PC0-PC3)
- **Software Implementation**: IRQ-based input detection with event routing
  - FreeRTOS software timers created for future debounce implementation
  - Event routing through EventDispatcher to UIController working correctly
  - UART debug traces for interrupt verification
- **Current Status**: Button detection functional, debounce logic pending completion
- **Files Modified**: `RETRO_GAME_proj.ioc`, `main.c/h`, `InputDrv.c/h`

#### ✅ TFT Display Initialization Fix
- **Problem**: `tftInit()` hanging during boot due to `HAL_Delay()` calls before FreeRTOS scheduler start
- **Solution**: Implemented custom `busyDelay()` function using CPU cycle counting
  ```c
  static void busyDelay(uint32_t ms) {
      volatile uint32_t count = ms * 21000; // 84MHz clock approximation
      while(count--) { __NOP(); }
  }
  ```
- **Files Modified**: `ScreenDrv.c`

#### ✅ LVGL Graphics Library Integration
- **Achievement**: Complete graphics system implementation with LVGL library
- **Implementation**: Robust task synchronization and memory optimization
  - Created binary semaphores for GraphEngine ↔ UIController coordination
  - Two-phase initialization: LVGL setup → UI creation → main loop
  - Integrated LVGL with FreeRTOS memory management (custom realloc implementation)
  - LVGL timer now uses FreeRTOS tick hook instead of independent timing
  - **Critical**: `configUSE_TICK_HOOK 1` required for LVGL operation - enables `lv_tick_inc()` and internal timer processing
  - Created custom RTOS porting layer for LVGL-FreeRTOS integration
- **Files Added**: `lv_port_rtos.c/h` (FreeRTOS tick hook implementation)
- **Files Modified**: `GraphEngine.c`, `UIController.c`, `synchronization.h` (renamed from `events.h`), `FreeRTOSConfig.h`

#### ✅ FreeRTOS Memory Optimization
- **Problem**: Heap corruption and stack overflow issues with 128KB RAM constraint
- **Solution**: Comprehensive memory allocation optimization
  - Increased heap size: 15KB → 25KB (`configTOTAL_HEAP_SIZE`)
  - Optimized task stack sizes:
    - GraphEngine: 1× → 6× (LVGL rendering requirements)
    - UIController: 1× → 4× (UI creation and management)
    - Other tasks: reduced to 1× (minimal requirements)
  - Added stack monitoring with `uxTaskGetStackHighWaterMark()`
- **Files Modified**: `FreeRTOSConfig.h`, `BootMng.c`

#### ✅ Task Synchronization Implementation
- **Problem**: Race conditions during LVGL initialization causing Hard Faults
- **Solution**: Implemented proper synchronization patterns
  ```c
  // GraphEngine: Phase 1 - Initialize LVGL
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();
  xSemaphoreGive(semGraphEngineReady);
  
  // UIController: Wait for LVGL ready, then create UI
  xSemaphoreTake(semGraphEngineReady, portMAX_DELAY);
  ui_init();
  xSemaphoreGive(semUiReady);
  
  // GraphEngine: Phase 2 - Main rendering loop
  xSemaphoreTake(semUiReady, portMAX_DELAY);
  // Main LVGL timer handler loop
  ```
- **Files Added**: `synchronization.h`
- **Files Modified**: `GraphEngine.c`, `UIController.c`, `BootMng.c`

### Current System Status

#### ✅ Working Features
- **Graphics System**: LVGL UI rendering with DMA-accelerated display driver and FreeRTOS integration
- **Input System**: 4 buttons (A/B/C/D) with IRQ-based detection + 2-axis analog joystick working correctly
- **UI Navigation**: Menu system fully functional with button and joystick input
- **Memory Management**: Stable operation with optimized heap and stack allocation
- **Task Coordination**: Proper synchronization between all system components with event routing
- **Audio Infrastructure**: PWM-based audio driver ready for implementation
- **Event System**: Event dispatcher routing input events to UI controller
- **FreeRTOS Integration**: Timer deamon task configured for freeRTOS software timers.

#### 📝 Known Minor Issues
- Brief display of previous screen content before splash screen (cosmetic)
- Screen clearing now implemented but legacy content may still flash briefly

### Technical Specifications
- **MCU**: STM32F446RE (Cortex-M4, 180MHz, 128KB RAM, 512KB Flash)
- **RTOS**: FreeRTOS with CMSIS-RTOS v1 wrapper
- **Graphics**: LVGL v8.x with ST7735 TFT display (128x160 RGB565)
- **Memory**: 25KB heap, optimized task stacks, stack monitoring enabled
- **Audio**: TIM3/TIM6 PWM with DMA1_Stream1 circular buffer
- **Input**: 4 digital buttons + ADC dual-channel joystick

### Development Environment
- **IDE**: STM32CubeIDE with HAL drivers
- **Configuration**: STM32CubeMX `.ioc` project file
- **Debugging**: UART trace output, FreeRTOS queue registry, stack monitoring

---

## [Beta 0.1] - Previous
- Initial project structure and basic component framework
- Event dispatcher and basic task architecture
- Driver interfaces and system initialization

---

### Notes for Future Development
- Audio playback system ready for sound effect and music implementation  
- Game state management framework in place for game logic integration
- Persistence layer available for save/load functionality
- System is stable and ready for game content development