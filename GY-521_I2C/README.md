# Device driver para GY-521 en STM32


## GY-521 (MPU-6500)

[Datasheet](./docs/PS-MPU-6500A-01-v1.3.pdf)
[Register Map](./docs/MPU-6500-Register-Map2.pdf)

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