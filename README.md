# i2c_beacon_lp

Some boiler plate code to show abstracting BLE services to a different file in NCS/Zephyr ecosystem, retrieving i2c sensor data from a device and then advertising it as custom characteristics.
It also utilizes device power management API with the i2c device for when it is not needed.

For more details on exchange in BLE (characteristics, etc), visit this section in the [Nordic DevAcademy]([url](https://academy.nordicsemi.com/courses/bluetooth-low-energy-fundamentals/lessons/lesson-4-bluetooth-le-data-exchange/)).

File | Function
--- | ---
main.c | main application
si7021.c | si7021 sensor inits, commands, and power management for i2c device
services/* | shs.c/h (sensorhubservice) for BLE service to be able to read humidity and temperature from phone

| Compatible devices|
|---|
| nRF54L15DK*** |
| nRF52832DK |
| nRF52840DK*|
| nRF5340DK**|
> *should work, haven't tried.
> 
> **need to change i2c instance from 0 to 1.
> 
> ***need to change i2c instance to 22

## hardware / documentation
- nRF54L15DK / [nRF54L15 doc](https://docs.nordicsemi.com/bundle/ps_nrf54L15/page/keyfeatures_html5.html), [nRF54L15DK doc](https://docs.nordicsemi.com/bundle/ug_nrf54l15_dk/page/UG/nRF54L15_DK/intro/intro.html)
- Adafruit Si7021 / [Si7021 doc](https://learn.adafruit.com/adafruit-si7021-temperature-plus-humidity-sensor/downloads)

![image](https://github.com/user-attachments/assets/5d618d76-0613-45c1-8b92-e4872182b46c)

![image](https://github.com/user-attachments/assets/36c66a18-5e2b-4c88-ae67-0479ae19e857)



> (nice little sensor pack)


# Power Management
[Zephyr Power Management](https://docs.zephyrproject.org/2.7.0/reference/power_management/index.html)

[Device Power Management](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/device.html)

[Device Runtime Power Management](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/device_runtime.html#pm-device-runtime)

[Device Power Management Implementation Guidelines](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/device_runtime.html#implementation-guidelines)

[nRF5x System Off sample](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.0/zephyr/samples/boards/nrf/system_off/README.html)

## Kconfigs
```
CONFIG_PM: This option enables the board to implement extra power management
policies whenever the kernel becomes idle. The kernel informs the
power management subsystem of the number of ticks until the next kernel
timer is due to expire.

CONFIG_PM_DEVICE: This option enables the device power management interface.  The
interface consists of hook functions implemented by device drivers
that get called by the power manager application when the system
is going to suspend state or resuming from suspend state. This allows
device drivers to do any necessary power management operations
like turning off device clocks and peripherals. The device drivers
may also save and restore states in these hook functions.

CONFIG_PM_DEVICE_RUNTIME: Enable Runtime Power Management to save power.
With device runtime PM enabled, devices can be suspended or resumed based on the device
usage even while the CPU or system is running.
```

# System power management
Looking at the [code for the nRF5340's power management](https://github.com/zephyrproject-rtos/zephyr/blob/69d0dce978e11a17e6605a42067374ca56767483/soc/arm/nordic_nrf/nrf53/power.c#L10-L28), you can see that the only supported power management state is `PM_STATE_SOFT_OFF`.

To wake from `PM_STATE_SOFT_OFF`, you can use a gpio, nfc, or [lpcomp](https://infocenter.nordicsemi.com/topic/ps_nrf5340/lpcomp.html?resultof=%22%6c%70%63%6f%6d%70%22%20).

Review: https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/system.html

##### system pm enum
```
PM_STATE_ACTIVE
    Runtime active state.
    The system is fully powered and active.

PM_STATE_RUNTIME_IDLE
    Runtime idle state.
    Runtime idle is a system sleep state in which all of the cores enter deepest possible idle state and wait for interrupts, no requirements for the devices, leaving them at the states where they are.

PM_STATE_SUSPEND_TO_IDLE
    Suspend to idle state.
    The system goes through a normal platform suspend where it puts all of the cores in deepest possible idle state and may puts peripherals into low-power states. No operating state is lost (ie. the cpu core does not lose execution context), so the system can go back to where it left off easily enough.

PM_STATE_STANDBY
    Standby state.
    In addition to putting peripherals into low-power states all non-boot CPUs are powered off. It should allow more energy to be saved relative to suspend to idle, but the resume latency will generally be greater than for that state. But it should be the same state with suspend to idle state on uniprocesser system.

PM_STATE_SUSPEND_TO_RAM
    Suspend to ram state.
    This state offers significant energy savings by powering off as much of the system as possible, where memory should be placed into the self-refresh mode to retain its contents. The state of devices and CPUs is saved and held in memory, and it may require some boot- strapping code in ROM to resume the system from it.

PM_STATE_SUSPEND_TO_DISK
    Suspend to disk state.
    This state offers significant energy savings by powering off as much of the system as possible, including the memory. The contents of memory are written to disk or other non-volatile storage, and on resume it’s read back into memory with the help of boot-strapping code, restores the system to the same point of execution where it went to suspend to disk.

PM_STATE_SOFT_OFF
    Soft off state.
    This state consumes a minimal amount of power and requires a large latency in order to return to runtime active state. The contents of system(CPU and memory) will not be preserved, so the system will be restarted as if from initial power-up and kernel boot.
```

# Device power management


![image](https://github.com/droidecahedron/i2c_ble_peripheral/assets/63935881/61dfdce8-a610-408b-a568-07e6d3a1b525)
*from https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/device.html#device-power-management-states*

Each driver supports certain power management modes as well. 
- [i2c twi](https://github.com/zephyrproject-rtos/zephyr/blob/fd346e846f1474468cd2bd67e7208b9560edd60e/drivers/i2c/i2c_nrfx_twi.c#L234-L269)
- [i2c twim](https://github.com/zephyrproject-rtos/zephyr/blob/fd346e846f1474468cd2bd67e7208b9560edd60e/drivers/i2c/i2c_nrfx_twim.c#L304-L337)

(In the case of i2c twi/twim, they only have `PM_DEVICE_ACTION_RESUME` and `PM_DEVICE_ACTION_SUSPEND`)
> The TWIM peripheral uses DMA, and the TWI peripheral does not. That is not the only difference, but it is the most significant.



## Extra Documentation
> [Measuring current on the DK](https://infocenter.nordicsemi.com/topic/ug_nrf5340_dk/UG/dk/hw_measure_current.html?cp=4_0_4_4)

> [nRF53 Power Optimization Blog](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/optimizing-power-on-nrf53-designs)

> [Blog on NCS misconception has power measurement section](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/debunking-misconceptions-a-technical-analysis-of-nrf5-sdk-and-nrf-connect-sdk)


# Screenshots
PM on i2c, barely any optimizations, just removing logging, making advertising slow (10 second interval), gpio still present in this...

## While connected
![image](https://github.com/user-attachments/assets/3c399c40-199c-41a7-a6b0-7e5324414d2b)

## While advertising
![image](https://github.com/user-attachments/assets/12875418-2573-4624-90e2-5b3cd66e513d)

Can optimize a lot further, but 235 mAh / 0.05 mA lasts a pretty long time.


![image](https://github.com/droidecahedron/i2c_ble_peripheral/assets/63935881/83e01ada-7f7f-4d79-a479-23c0a79e5002)

on phone side:

![image](https://github.com/droidecahedron/i2c_ble_peripheral/assets/63935881/ca1de9c6-9ebd-43f1-86ab-cafbc68183ab) ![image](https://github.com/droidecahedron/i2c_ble_peripheral/assets/63935881/32dac790-caf8-442c-bf25-ff972344f4b4)

# optimizing further
you can add a deep sleep mode.
This sensor board also has an onboard regulator to supply power and I'm not turning it off in software, so you could get a better sensor.

# debugging
The settings are set up to flash and measure, active development will be annoying with the default settings. See below to make it more convenient: 

- re-enable logging and UART via `prj.conf`, it's a power hit.
- lower the advertising interval, it's presently at the max. You don't connect very quickly with the max setting.
