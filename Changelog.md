# Changelog

## Unreleased (Post-Beta 0.3)

### Driver Architecture Improvements

#### Hardware Abstraction - Normalized Output Values
- **Standardized driver output**: All input drivers now return normalized values (-1024..+1023 range, 2048 total values)
  - **AccelDrv**: Returns -1024..+1023 (±1G ≈ ±1024) instead of raw ±16384 LSB values
  - **InputDrv joystick**: Returns -1024..+1023 (center = 0) instead of raw 0-4095 ADC values
  - **Range rationale**: 2048 values (2^11) provides good resolution while being power-of-2 aligned
  - **Benefits**:
    - Hardware-independent application code
    - No knowledge of ADC resolution or sensor scaling required by consumers
    - Easy sensor replacement without application code changes
    - Consistent interface across different input sources
    - Better resolution than ±100 for fine control (10x more precision)

- **AccelDrv normalization**:
  - Applied after EMA filtering and deadband in `processAccelData()`
  - Formula: `normalized = (raw * 1024) / 16384`
  - Clamped to -1024..+1023 range before publishing
  - Updated API documentation in AccelDrv.h

- **InputDrv normalization**:
  - Applied in `updateJoystickPublishedValues()` after EMA filtering
  - Formula: `normalized = ((adc - 2048) * 2048) / 4095`
  - Clamped to -1024..+1023 range before publishing
  - Internal directional FSM still uses raw ADC values for hysteresis
  - Changed `joystick_t` fields from `uint16_t` to `int16_t`
  - Updated API documentation in InputDrv.h

- **InputHandler simplification**:
  - Removed normalization helper functions (no longer needed)
  - Now uses driver values directly: `act.pitch = tilt.tiltX;`
  - Cleaner code with single responsibility (game logic, not data conversion)

### Bug Fixes

#### InputDrv - Joystick Direction Mapping
- **Fixed inverted Y-axis direction**: Corrected mapping in `getDominantDirection()` function
  - Up movement now correctly generates `INP_JY_UP` event (was generating `INP_JY_DOWN`)
  - Down movement now correctly generates `INP_JY_DOWN` event (was generating `INP_JY_UP`)
  - Changed: `(dy >= 0) ? JOY_KEY_DOWN : JOY_KEY_UP` → `(dy >= 0) ? JOY_KEY_UP : JOY_KEY_DOWN`
  - File: InputDrv.c:364

### TimerDrv - Timing Utilities

#### Pre-RTOS Delay Function
- **Centralized busy-wait delay**: Moved delay logic from individual drivers to TimerDrv utilities
  - `timerDelayMs()`: CPU cycle-based delay for pre-RTOS initialization
  - Calibrated for 84MHz STM32F446RE (21000 iterations ≈ 1ms)
  - Prevents code duplication across multiple drivers
  - Used by HapticDrv, AccelDrv, and ScreenDrv during initialization before scheduler starts
- **Implementation details**:
  - Volatile counter prevents compiler optimization
  - `__NOP()` instruction ensures consistent timing
  - Works reliably before FreeRTOS scheduler initialization
- **Removed**:
  - `timerInit()` function (no longer needed, TimerDrv is now utilities-only)
  - `hapticDelay()` from HapticDrv.c (replaced with `timerDelayMs()`)
  - `tftDelay()` from ScreenDrv.c (replaced with `timerDelayMs()`)
  - Direct `HAL_Delay()` calls from AccelDrv calibration (replaced with `timerDelayMs()`)

### Haptic Feedback System Implementation

#### HapticDrv - Low-Level Driver
- **DRV2605 I2C driver**: Complete hardware abstraction for Texas Instruments DRV2605 haptic motor controller
  - I2C communication with 10ms timeout and error handling
  - Soft reset and initialization sequence for ERM (Eccentric Rotating Mass) motors
  - Waveform library selection (6 available libraries: empty, 5 ERM variants, 1 LRA)
  - 8-slot waveform sequencer with 123 pre-programmed effects
  - Internal trigger mode operation with real-time playback support
  - Uses `timerDelayMs()` for soft reset timing during initialization
- **API functions**:
  - `hapticInit()`: Initialize device with ERM motor configuration
  - `hapticSelectLibrary()`: Select waveform library (1-6)
  - `hapticSetWaveform()`: Program individual sequencer slot (0-7)
  - `hapticSetSequence()`: Program complete 8-step sequence
  - `hapticGoSequence()` / `hapticStopSequence()`: Playback control
  - `hapticTrigger()`: Single-effect convenience function

#### HapticEngine - Feedback Engine
- **Pattern management system**: High-level abstraction for haptic feedback patterns
  - 9 predefined haptic patterns stored in lookup table
  - Pattern definitions using DRV2605 waveform IDs from datasheet
  - Abstraction layer between system events and low-level driver
- **Haptic patterns**:
  - `HP_LIGHT_CLICK`: Soft single click (waveform 14)
  - `HP_MEDIUM_CLICK`: Medium strength click (waveform 1 - Strong Click 100%)
  - `HP_STRONG_CLICK`: Strong single click (waveform 47)
  - `HP_DOUBLE_CLICK`: Double click pattern (14, 14)
  - `HP_TRIPLE_CLICK`: Triple click pattern (14, 14, 14)
  - `HP_SOFT_BUZZ`: Soft continuous buzz (waveform 17)
  - `HP_ALERT_BUZZ`: Alert buzz pattern (10, 10, 10)
  - `HP_SUCCESS_PULSE`: Success confirmation (15, 16)
  - `HP_ERROR_BUZZ`: Error indication (58, 58, 58)
- **Event-driven architecture**: FreeRTOS task consuming `qHaptic` queue
  - Receives `HAP_*` events from EventDispatcher
  - Maps events to haptic patterns via switch statement
  - Non-blocking pattern playback using DRV2605 internal trigger mode

#### System Integration
- **EventDispatcher routing**: Added routing for 9 haptic event types
  - `HAP_LIGHT_CLICK` through `HAP_ERROR_BUZZ` routed to `qHaptic` queue
  - Zero-delay queue send for real-time responsiveness
- **UIController haptic feedback**: Integrated haptic events with user input
  - Button A → `HAP_LIGHT_CLICK`
  - Button B → `HAP_MEDIUM_CLICK`
  - Button C → `HAP_STRONG_CLICK`
  - Button D → `HAP_DOUBLE_CLICK`
  - Joystick directional navigation → `HAP_SOFT_BUZZ`
  - Haptic events sent alongside audio events (`AUP_BEEP_*`)
- **Event definitions**: Added 9 haptic events to `synchronization.h` (lines 105-113)
- **Boot sequence integration**: HapticDrv initialized in `BootMng.c` during hardware setup phase

#### Technical Architecture
- **Multi-layer design**:
  1. HapticDrv: Hardware abstraction (I2C, registers, waveforms)
  2. HapticEngine: Pattern management and event processing
  3. EventDispatcher: Event routing from system to haptic queue
  4. UIController: User interaction feedback generation
- **ISR-safe operation**: All event generation uses FreeRTOS ISR-safe functions
- **Non-blocking**: Haptic playback uses DRV2605 autonomous sequencer, no CPU blocking

### Accelerometer Driver Implementation

#### AccelDrv - MPU-6500 Driver
- **MPU-6500/GY-521 I2C DMA driver**: Complete hardware abstraction for MPU-6500 accelerometer
  - Device ID verification (WHO_AM_I = 0x70)
  - I2C address support: 0x68 (AD0=LOW) or 0x69 (AD0=HIGH)
  - DMA-based continuous reading triggered by timer
  - Non-blocking transfers with automatic watchdog protection
  - 6-byte burst read (X, Y, Z axes in 16-bit big-endian format)
- **Low-Noise mode configuration**: Optimized for clean and stable signal
  - Current consumption: ~450µA
  - Internal sampling: 1 kHz
  - DLPF (Digital Low-Pass Filter): 94 Hz cutoff
  - Measurement range: ±2G (sensitivity 16384 LSB/G)
  - Gyroscope and temperature sensor disabled for power savings
  - Registers configured: PWR_MGMT_1=0x08, PWR_MGMT_2=0x07, ACCEL_CONFIG=0x00, ACCEL_CONFIG_2=0x02
- **Real-time filtering pipeline**:
  - **EMA filter** (Exponential Moving Average): α=0.5 (Q15 format)
  - **Deadband filter**: ±100 counts (±0.006G) to eliminate drift and micro-vibrations
  - **Auto-calibration**: 100-sample averaging at initialization (assumes device stationary and level)
  - **Offset compensation**: Calibrates X, Y, and Z-1G to center at zero
  - Uses `timerDelayMs()` for calibration sample timing (10ms between samples)
- **DMA watchdog protection**:
  - Timeout: 5 timer ticks (50ms @ 100Hz)
  - Auto-recovery from I2C bus lockups
  - Aborts stalled transfers and resets state for automatic retry
- **Hardware configuration**:
  - **I2C1**: Standard mode (100 kHz) with DMA1 Stream 0 Channel 1 for RX
  - **TIM4**: Trigger frequency 100 Hz (PSC=8340-1, ARR=100-1)
  - **DMA**: Normal mode (non-circular), byte transfers, medium priority
  - **Interrupts**: TIM4_IRQn, DMA1_Stream0_IRQn
  - **Note**: I2C1_ER_IRQn (error interrupt) currently disabled for protoboard development due to false positives from unstable connections. DMA watchdog provides sufficient error recovery. Re-enable for PCB production.
- **API functions**:
  - `accelDrvInit()`: Initialize MPU-6500, calibrate, and start continuous DMA reads
  - `accelDrvGetTilt()`: Non-blocking read of filtered tilt values (thread-safe)
- **HAL callbacks** (auto-implemented in AccelDrv.c):
  - `HAL_TIM_PeriodElapsedCallback()`: Triggers DMA read with watchdog check
  - `HAL_I2C_MemRxCpltCallback()`: Processes received data (filtering + publishing)
  - `HAL_I2C_ErrorCallback()`: Enables automatic retry on I2C errors

#### Performance Characteristics
- **Frequencies**:
  - Timer trigger rate: 100 Hz (every 10ms)
  - Sensor internal sampling: 1 kHz (LOW_NOISE mode)
  - DLPF bandwidth: 94 Hz
  - Effective output rate: 100 Hz (limited by timer)
- **Latency**: From timer trigger to data available < 1ms
  - DMA transfer time @ 100kHz I2C: ~600µs (6 bytes)
- **Memory footprint**:
  - Static RAM: ~30 bytes (buffers and filter state)
  - Flash: ~2 KB (driver code)
  - Stack: 0 bytes (no dedicated task)
  - Heap: 0 bytes (no dynamic allocations)

#### Data Structure
```c
typedef struct {
    int16_t tiltX;  // X-axis tilt (calibrated, filtered) ±16384 for ±1G
    int16_t tiltY;  // Y-axis tilt (calibrated, filtered) ±16384 for ±1G
} accelTilt_t;
```

#### Game Integration
- **InputHandler modification**: Replaced joystick reading with accelerometer input
  - Accelerometer data read via `accelDrvGetTilt()` at 50Hz (GFX_TICK_MS)
  - Normalization function `norm_accel_to_100()`: Maps ±16384 (±1G) to ±100
  - **Control mapping**:
    - `tiltX` (X-axis) → pitch (horizon height control)
    - `tiltY` (Y-axis) → roll (lateral tilt)
  - Accelerometer values flow through existing game pipeline: InputHandler → qActions → GameManager → RenderEngine → GraphEngine

---

## Beta 0.3 (Released)

See README.md for complete Beta 0.3 changelog.
