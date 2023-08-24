/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/logging/log.h>

#include "inc/si7021.h"


#define SLEEP_TIME_MS   5000


LOG_MODULE_REGISTER(i2c_ble_p);

void main(void)
{

    int ret;
    float humidity;
    float temperature_C;

    si7021_init();
    k_msleep(100); // give it some time after resetting.



    while (1) {
        humidity = si7021_read_humidity();
        temperature_C = si7021_read_temperature();

        LOG_INF("Humidity: %.2f, Temperature (C) : %.2f", humidity, temperature_C);
        k_msleep(SLEEP_TIME_MS);
    }
}