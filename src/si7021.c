#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/devicetree.h>

#include <zephyr/logging/log.h>
#include <zephyr/drivers/i2c.h>

// power management
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>

#include "inc/si7021.h"

LOG_MODULE_REGISTER(si7021);

#define I2C_NODE DT_NODELABEL(si7021)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);
const struct device *i2c_device = DEVICE_DT_GET(DT_NODELABEL(i2c1));

bool si7021_init(void)
{
    int err;
    if (!device_is_ready(dev_i2c.bus))
    {
        printk("I2C bus %s is not ready!\n\r", dev_i2c.bus->name);
        return false;
    }

    uint8_t cmd[] = {SI7021_RESET_CMD};
    err = i2c_write_dt(&dev_i2c, cmd, sizeof(cmd));
    if (err != 0)
    {
        LOG_WRN("Failed to write to i2c dev addr\n");
        return false;
    }

    //power management
    err = pm_device_action_run(i2c_device, PM_DEVICE_ACTION_SUSPEND);

    return true;
}

float si7021_read_humidity(void)
{
    int err;
    float relative_humidity;

    err = pm_device_action_run(i2c_device, PM_DEVICE_ACTION_RESUME);

    if (!device_is_ready(dev_i2c.bus))
    {
        LOG_WRN("I2C bus %s is not ready!\n\r", dev_i2c.bus->name);
        return false;
    }

    // start conversion
    uint8_t cmd[] = {SI7021_MEASRH_NOHOLD_CMD};
    uint8_t buf[2] = {0};
    err = i2c_write_dt(&dev_i2c, cmd, sizeof(cmd));
    if (err != 0)
    {
        LOG_WRN("Failed to write/read to i2c dev\n");
        return SI7021_ERRCODE;
    }
    k_msleep(20); // conversion time

    err = i2c_read_dt(&dev_i2c, buf, sizeof(buf));
    if (err != 0)
    {
        LOG_WRN("Failed to write/read to i2c dev\n");
        return SI7021_ERRCODE;
    }
    
    err = pm_device_action_run(i2c_device, PM_DEVICE_ACTION_SUSPEND);

    relative_humidity = (125 * ((buf[0] << 8) | buf[1]) / 65536) - 6;
    return relative_humidity;
}

float si7021_read_temperature(void)
{
    int err;
    float temperature_C;

    err = pm_device_action_run(i2c_device, PM_DEVICE_ACTION_RESUME);

    if (!device_is_ready(dev_i2c.bus))
    {
        printk("I2C bus %s is not ready!\n\r", dev_i2c.bus->name);
        return false;
    }

    // start conversion
    uint8_t cmd[] = {SI7021_MEASTEMP_NOHOLD_CMD};
    uint8_t buf[2] = {0};
    err = i2c_write_dt(&dev_i2c, cmd, sizeof(cmd));
    if (err != 0)
    {
        LOG_WRN("Failed to write/read to i2c dev\n");
        return SI7021_ERRCODE;
    }

    k_msleep(20); // conversion time

    err = i2c_read_dt(&dev_i2c, buf, sizeof(buf));
    if (err != 0)
    {
        LOG_WRN("Failed to write/read to i2c dev\n");
        return SI7021_ERRCODE;
    }

    err = pm_device_action_run(i2c_device, PM_DEVICE_ACTION_SUSPEND);

    //unit conversion
    temperature_C = (175.72 * ((buf[0] << 8) | buf[1]) / 65536) - 46.85;
    return temperature_C;
}
