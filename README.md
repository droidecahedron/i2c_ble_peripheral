# i2c_beacon_lp

# TODO:
`pm_device_state_set` was replaced with `pm_device_action_run`

ex:
```c
//wake up
pm_device_action_run(dev, PM_DEVICE_ACTION_RESUME)
//sensor stuff
pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND)
```
> ./zephyr/doc/releases/release-notes-3.0.rst:794

| Compatible devices|
|---|
| nRF52840DK*|
| nRF5340DK|
> *should work, haven't tried.

## hardware / documentation
- nRF5340DK / [nRF5340 doc](https://infocenter.nordicsemi.com/topic/struct_nrf53/struct/nrf5340.html), [nRF5340DK doc](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_nrf53%2Fstruct%2Fnrf5340.html)
- Adafruit Si7021 / [Si7021 doc](https://learn.adafruit.com/adafruit-si7021-temperature-plus-humidity-sensor/downloads)

<img src="https://github.com/droidecahedron/nrf-blueberry/assets/63935881/12612a0e-9f81-4431-8b22-f69704248f89" width=25% height=25%>
<img src="https://github.com/droidecahedron/i2c_beacon_lp/assets/63935881/771948d8-2f2b-4309-9ebe-93af712902df", width=22% height=22%>


> (nice little sensor pack)


## Power Management
[Zephyr Power Management](https://docs.zephyrproject.org/2.7.0/reference/power_management/index.html)

[Device Power Management](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/device.html)

[Device Runtime Power Management](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/device_runtime.html#pm-device-runtime)

[Device Power Management Implementation Guidelines](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/services/pm/device_runtime.html#implementation-guidelines)

[nRF5x System Off sample](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.0/zephyr/samples/boards/nrf/system_off/README.html)

### Kconfigs
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

PM_DEVICE_RUNTIME
	bool "Runtime Device Power Management"
	select EVENTS
	help
	  Enable Runtime Power Management to save power. With device runtime PM
	  enabled, devices can be suspended or resumed based on the device
	  usage even while the CPU or system is running.
```

### System power management
Looking at the [code for the nRF5340's power management](https://github.com/zephyrproject-rtos/zephyr/blob/69d0dce978e11a17e6605a42067374ca56767483/soc/arm/nordic_nrf/nrf53/power.c#L10-L28), you can see that the only supported power management state is `PM_STATE_SOFT_OFF`.

To wake from `PM_STATE_SOFT_OFF`, you can use a gpio, nfc, or [lpcomp](https://infocenter.nordicsemi.com/topic/ps_nrf5340/lpcomp.html?resultof=%22%6c%70%63%6f%6d%70%22%20).

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

### Device power management
Each driver supports certain power management modes as well. 
- [i2c twi](https://github.com/zephyrproject-rtos/zephyr/blob/fd346e846f1474468cd2bd67e7208b9560edd60e/drivers/i2c/i2c_nrfx_twi.c#L234-L269)
- [i2c twim](https://github.com/zephyrproject-rtos/zephyr/blob/fd346e846f1474468cd2bd67e7208b9560edd60e/drivers/i2c/i2c_nrfx_twim.c#L304-L337)

##### device pm enum
```
PM_DEVICE_STATE_ACTIVE
    device is in ACTIVE power state
    Normal operation of the device. All device context is retained.

PM_DEVICE_STATE_LOW_POWER
    device is in LOW power state
    Device context is preserved by the HW and need not be restored by the driver.

PM_DEVICE_STATE_SUSPEND
    device is in SUSPEND power state
    Most device context is lost by the hardware. Device drivers must save and restore or reinitialize any context lost by the hardware

PM_DEVCIE_STATE_FORCE_SUSPEND
    device is in force SUSPEND power state
    Driver puts the device in suspended state after completing the ongoing transactions and will not process any queued work or will not take any new requests for processing. Most device context is lost by the hardware. Device drivers must save and restore or reinitialize any context lost by the hardware.

PM_DEVICE_STATE_OFF
    device is in OFF power state
    Power has been fully removed from the device. The device context is lost when this state is entered, so the OS software will reinitialize the device when powering it back on
```

(In the case of i2c twi/twim, they only have `PM_DEVICE_ACTION_RESUME` and `PM_DEVICE_ACTION_SUSPEND`)
> The TWIM peripheral uses DMA, and the TWI peripheral does not. That is not the only difference, but it is the most significant.



## Extra Documentation
> [Measuring current on the DK](https://infocenter.nordicsemi.com/topic/ug_nrf5340_dk/UG/dk/hw_measure_current.html?cp=4_0_4_4)

> [nRF53 Power Optimization Blog](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/optimizing-power-on-nrf53-designs)

> [Blog on NCS misconception has power measurement section](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/debunking-misconceptions-a-technical-analysis-of-nrf5-sdk-and-nrf-connect-sdk)


# Screenshots
No i2c, barely any optimizations, just removing logging, making advertising slow (10 second interval), gpio still present in this...
![image](https://github.com/droidecahedron/i2c_beacon_lp/assets/63935881/861f06a2-97ab-45e7-86a9-92c02f2d67a0)


Can optimize a lot further.
