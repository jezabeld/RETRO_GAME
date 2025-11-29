# GY-521 Accelerometer Driver with I2C DMA

Driver for the MPU-6500 accelerometer of the GY-521 module, implemented with continuous I2C DMA reading.

[Datasheet](./docs/PS-MPU-6500A-01-v1.3.pdf)
[Register Map](./docs/MPU-6500-Register-Map2.pdf)

## Driver Features

### Architecture
- **DMA reading**: Non-blocking I2C transfers triggered by timer
- **Real-time filtering**: EMA (Exponential Moving Average) + deadband
- **Integrated watchdog**: DMA hang protection with automatic timeout
- **Simple API**: Only 2 public functions

### Sensor Configuration (LOW_NOISE Mode)

**Selected mode**: `ACCEL_LOW_NOISE_MODE`

**LOW_NOISE characteristics:**
- Current consumption: ~450µA
- Internal sampling frequency: 1 kHz
- DLPF (Digital Low-Pass Filter): Enabled with fc=94Hz
- Gyroscope: Disabled
- Temperature: Disabled
- Clock: Internal oscillator
- Advantages: Clean and stable signal, fast response

**Configured registers:**
```c
REG_PWR_MGMT_1 = 0x08  // TEMP_DIS | CLKSEL_INTERNAL
REG_PWR_MGMT_2 = 0x07  // GYRO_OFF
REG_ACCEL_CONFIG = 0x00  // ±2G range (default)
REG_ACCEL_CONFIG_2 = 0x02  // DLPF cfg=2 (94Hz BW, 1kHz rate)
```

**Measurement range**: ±2G (sensitivity 16384 LSB/G)

### DMA Configuration

**I2C1 Peripheral:**
- DMA Channel: DMA1 Stream 0 Channel 1
- Direction: RX (I2C1_RX)
- Mode: Normal (non-circular)
- Data size: Byte
- Buffer: 6 bytes (XH, XL, YH, YL, ZH, ZL)
- DMA Priority: Medium
- DMA Interrupt: Enabled (Transfer Complete)

**Timer TIM4:**
- Frequency: 100 Hz (period = 10 ms)
- Prescaler: 8340-1 (84 MHz → 10 kHz)
- Period: 100-1 (10 kHz → 100 Hz)
- Interrupt: Enabled (Update Event)

**Configured interrupts:**
- `TIM4_IRQn`: Triggers DMA read every 10ms
- `DMA1_Stream0_IRQn`: Notifies DMA transfer completion
- `I2C1_ER_IRQn`: Handles I2C errors (recommended for robustness)

### Data Filtering

**EMA Filter (Exponential Moving Average):**
- Alpha (Q15): 0.5 (16384/32768)
- Equation: `y = y + α(x - y)`
- Advantage: Smooths noise while maintaining fast response

**Deadband:**
- Threshold: ±100 counts (~±0.006G)
- Applied after EMA
- Eliminates drift and micro-vibrations at rest

**Calibration:**
- Automatic at initialization
- 100 averaged samples
- Assumes device stationary and level
- Compensates offset in all 3 axes (X, Y, Z-1G)

### Hang Protection

**DMA Watchdog:**
- Timeout: 5 timer ticks (50ms @ 100Hz)
- Action: Aborts I2C transfer and resets flags
- Allows automatic retries on next tick
- Protects against I2C bus lockups

## Driver API

### Initialization

```c
#include "AccelJoystickDrv.h"

accelStatus_t accelDrvInit(I2C_HandleTypeDef *i2cHandle,
                           TIM_HandleTypeDef *timHandle,
                           uint8_t devAddress,
                           accelPowerMode_t mode);
```

**Parameters:**
- `i2cHandle`: I2C peripheral handle (e.g., `&hi2c1`)
- `timHandle`: Timer handle to trigger reads (e.g., `&htim4`)
- `devAddress`: 7-bit I2C address (typically `0x68`)
- `mode`: `ACCEL_LOW_NOISE_MODE` or `ACCEL_LOW_POWER_MODE`

**Return:**
- `ACCEL_OK`: Successful initialization
- `ACCEL_ERROR`: Initialization error

**Note:** The function automatically starts the timer. DMA readings begin immediately.

### Reading Values

```c
void accelDrvGetTilt(accelTilt_t *tilt);
```

**Parameters:**
- `tilt`: Pointer to structure that will receive the filtered values

**Return structure:**
```c
typedef struct {
    int16_t tiltX;  // X-axis tilt (calibrated and filtered value)
    int16_t tiltY;  // Y-axis tilt (calibrated and filtered value)
} accelTilt_t;
```

**Characteristics:**
- Non-blocking read (returns last available value)
- Thread-safe on ARM Cortex-M (atomic 16-bit aligned access)
- Values already calibrated and filtered
- Typical range: ±16384 (±1G)

## HAL Integration

The driver requires 3 HAL callbacks that must be implemented in the project:

### 1. Timer Callback (stm32f4xx_it.c or main.c)

```c
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    // Driver callback (implemented in AccelJoystickDrv.c)
    // No additional user code required
}
```

### 2. I2C DMA Complete Callback (stm32f4xx_it.c or main.c)

```c
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    // Driver callback (implemented in AccelJoystickDrv.c)
    // No additional user code required
}
```

### 3. I2C Error Callback (stm32f4xx_it.c or main.c)

```c
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c) {
    // Driver callback (implemented in AccelJoystickDrv.c)
    // No additional user code required
}
```

**Important:** These callbacks are already implemented in `AccelJoystickDrv.c`. You only need to ensure they are not defined elsewhere in the project.

## Usage Example

```c
#include "main.h"
#include "AccelJoystickDrv.h"
#include "API_uart.h"

#define GY521_ADDR 0x68

int main(void) {
    HAL_Init();
    SystemClock_Config();

    // Initialize peripherals (I2C1, TIM4, DMA configured in CubeMX)
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2C1_Init();
    MX_TIM4_Init();

    // Initialize UART for debug
    uartInit();

    // Initialize accelerometer driver
    accelStatus_t status;
    status = accelDrvInit(&hi2c1, &htim4, GY521_ADDR, ACCEL_LOW_NOISE_MODE);

    if (status == ACCEL_OK) {
        uartSendString("Accelerometer initialized!\r\n");
    } else {
        uartSendString("ERROR: Accel init failed\r\n");
        while(1);  // Stop execution
    }

    accelTilt_t tilt;

    while (1) {
        // Non-blocking read
        accelDrvGetTilt(&tilt);

        // Use values for game control
        // tilt.tiltX: left/right tilt
        // tilt.tiltY: forward/backward tilt

        HAL_Delay(100);  // Update at 10Hz (optional)
    }
}
```

## STM32CubeMX Configuration

### I2C1 Configuration
1. Mode: I2C
2. I2C Speed: Standard Mode (100 kHz)
3. DMA Settings:
   - Add: I2C1_RX
   - DMA Request: I2C1_RX
   - Stream: DMA1 Stream 0
   - Direction: Peripheral to Memory
   - Priority: Medium
   - Mode: Normal
   - Data Width: Byte

### TIM4 Configuration
1. Clock Source: Internal Clock
2. Prescaler: 8340-1
3. Counter Period: 100-1
4. Auto-reload preload: Enable
5. NVIC Settings:
   - TIM4 global interrupt: Enable

### GPIO Configuration
1. PB8: I2C1_SCL (Alternate Function, Open Drain, Pull-up)
2. PB9: I2C1_SDA (Alternate Function, Open Drain, Pull-up)

### NVIC Configuration
Suggested priorities (lower number = higher priority):
- `DMA1_Stream0_IRQn`: Priority 0 (high)
- `I2C1_ER_IRQn`: Priority 1
- `TIM4_IRQn`: Priority 2

## Driver Files

```
GY-521_I2C/
├── Drivers/
│   ├── AccelJoystickDrv.h    # Public driver API
│   └── AccelJoystickDrv.c    # Complete implementation
├── Core/
│   ├── Src/
│   │   ├── main.c            # Integration example
│   │   ├── stm32f4xx_it.c    # Interrupt handlers
│   │   └── stm32f4xx_hal_msp.c  # MSP init for TIM4
└── README.md                 # This file
```

## Hardware Connection

**GY-521 Module:**
```
GY-521     STM32F446RE Nucleo
------     -------------------
VCC   -->  3.3V
GND   -->  GND
SCL   -->  PB8 (I2C1_SCL)
SDA   -->  PB9 (I2C1_SDA)
XDA   -->  (do not connect)
XCL   -->  (do not connect)
AD0   -->  GND (I2C address = 0x68)
INT   -->  (do not connect, not used)
```

**Notes:**
- GY-521 module includes 3.3V regulator and pull-ups on SCL/SDA
- AD0=GND → I2C address 0x68 (default)
- AD0=VCC → I2C address 0x69

## Troubleshooting

### Driver returns ACCEL_ERROR at initialization
- Verify I2C connections (SCL, SDA)
- Verify I2C address (0x68 or 0x69 depending on AD0)
- Verify 3.3V power supply to GY-521
- Use `HAL_I2C_IsDeviceReady()` for connection test

### Values are not updating
- Verify DMA is configured in CubeMX (I2C1_RX)
- Verify TIM4 interrupt is enabled
- Verify HAL callbacks are not duplicated in other files
- Check if watchdog is activating (add debug prints)

### Noisy or unstable values
- Increase EMA filter alpha (line 42 in AccelJoystickDrv.c)
- Increase deadband (line 43 in AccelJoystickDrv.c)
- Verify device is physically stable during calibration
- Re-calibrate by calling `accelDrvInit()` again

### High current consumption
- Change to `ACCEL_LOW_POWER_MODE` (consumption ~20µA vs ~450µA)
- Note: LOW_POWER has more noise and lower sampling frequency (40Hz)

## Technical Data

**Frequencies:**
- Timer trigger: 100 Hz (every 10 ms)
- Sensor sampling: 1 kHz internal (LOW_NOISE mode)
- DLPF bandwidth: 94 Hz
- Effective DMA rate: 100 Hz (limited by timer)

**Memory:**
- Stack: 0 bytes (no dedicated task)
- Heap: 0 bytes (no dynamic allocations)
- Static RAM: ~30 bytes (buffers and state)
- Flash: ~2 KB (driver code)

**Latency:**
- From timer trigger to data available: <1 ms
- DMA transfer time I2C @ 100kHz: ~600 µs (6 bytes)

## MPU-6500 Technical Documentation

### Device ID and I2C Address
- WHO_AM_I register: 0x70 (MPU-6500)
- I2C address: 0x68 (AD0=LOW) or 0x69 (AD0=HIGH)
- I2C address length: 7 bits (b110100X, X=AD0)

### Available Power Modes

| Mode | Gyro | Accel | Consumption |
| ---- | ---- | ----- | ----------- |
| Sleep Mode | Off | Off | Minimum |
| Standby Mode | Drive On | Off | Low |
| Low-Power Accelerometer Mode* | Off | Duty-Cycled | ~20µA |
| **Low-Noise Accelerometer Mode** | Off | **On** | **~450µA** |
| Gyroscope Mode | On | Off | High |
| 6-Axis Mode | On | On | Maximum |

**Implemented mode:** Low-Noise Accelerometer Mode

### Low-Power Mode Configuration (alternative)
If lower consumption is needed, configure:
- CYCLE bit = 1
- SLEEP bit = 0
- TEMP_DIS bit = 1
- DIS_XG, DIS_YG, DIS_ZG bits = 1

Wake-up frequencies (LP_WAKE_CTRL):
| LP_WAKE_CTRL | Frequency |
| ------------ | --------- |
| 0 | 1.25 Hz |
| 1 | 5 Hz |
| 2 | 20 Hz |
| 3 | 40 Hz |

### References
- [MPU-6500 Datasheet](./docs/PS-MPU-6500A-01-v1.3.pdf)
- [MPU-6500 Register Map](./docs/MPU-6500-Register-Map2.pdf)

## Author and License

**Project:** Portable Console - CESE Final Project
**Courses:** Communication Protocols in Embedded Systems and Microcontroller Programming
**Author:** Jez
**Date:** November 2025

**Hardware:**
- Board: STM32F446RE Nucleo
- Sensor: GY-521 (MPU-6500/MPU6050)

---

**Last updated:** November 29, 2025

*The  slave  address  of  the  MPU-6500  is  b110100X  which  is  7  bits  long. The  LSB  bit  of  the  7  bit  address  is determined by the logic level on pin AD0. The address of the divice should be b1101000 if pin AD0 is logic low.*

The contents of WHO_AM_I is an 8-bit device 
ID. The default value of the register is 0x70 for MPU-6500. This is different from the I2C address of 
the device as seen on the slave I2C controller by the applications processor. The I2C address of the 
MPU-6500 is 0x68 or 0x69 depending upon the value driven on AD0 pin.

SENSOR DATA REGISTERS  
The sensor data registers contain the latest gyro, accelerometer, auxiliary sensor, and temperature measurement 
data. They are read-only registers, and are accessed via the serial interface. Data from these registers may be read 
anytime. 

FIFO 
The MPU-6500 contains a 512-byte FIFO register that is accessible via the Serial Interface. The FIFO configuration 
register determines which data is written into the FIFO. Possible choices include gyro data, accelerometer data, 
temperature readings, auxiliary sensor readings, and FSYNC input. A FIFO counter keeps track of how many bytes of 
valid data are contained in the FIFO. The FIFO register supports burst reads. The interrupt function may be used to 
determine when new data is available.

STANDARD POWER MODES 
The following table lists the user-accessible power modes for MPU-6500. 

| Mode | Name | Gyro | Accel | DMP |
| ---- | ---- | ---- | ----- | --- | 
| 1 | Sleep Mode | Off|  Off|  Off| 
| 2 | Standby Mode | Drive On | Off| Off|  
| 3* | Low-Power Accelerometer Mode | Off | Duty-Cycled | Off | 
| 4 | Low-Noise Accelerometer Mode | Off | On | Off | 
| 5 | Gyroscope Mode | On | Off | On or Off | 
| 6 | 6-Axis Mode | On|  On | On or Off|  

(*) The MPU-6500 can be put into Accelerometer Only Low Power Mode using the following steps:  
- (i) Set CYCLE bit to 1 
- (ii) Set SLEEP bit to 0 
- (iii) Set TEMP_DIS bit to 1  
- (iv) Set DIS_XG, DIS_YG, DIS_ZG bits to 1

In  this  mode,  the  device  will  power  off  all  devices  except  for  the  primary  I2C  interface,  waking  only  
the  accelerometer at  fixed  intervals to take  a  single measurement.  The  frequency  of  wake-ups  can  
be configured with LP_WAKE_CTRL as shown below.  
| LP_WAKE_CTRL | Wake-up Frequency | 
| ------------ | ----------------- | 
| 0 | 1.25 Hz | 
| 1 | 5 Hz | 
| 2 | 20 Hz | 
| 3 | 40 Hz |



Note: The (max) accelerometer output rate is 1kHz.  This means that for a Sample Rate greater than 1kHz, 
the same accelerometer sample may be output to the  FIFO, DMP, and sensor  registers  more than 
once. 