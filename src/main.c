/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>


#include <zephyr/settings/settings.h>

#include "inc/si7021.h"
#include "services/ble_shs.h"

#define STACKSIZE 1024
#define PRIORITY 7

#define SENSOR_SLEEP_TIME_MS   15000

LOG_MODULE_REGISTER(main);

static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	(BT_LE_ADV_OPT_CONNECTABLE |
	 BT_LE_ADV_OPT_USE_IDENTITY), /* Connectable advertising and use identity address */
	16383, /* Min Advertising Interval 500ms (800*0.625ms) 16383*/
	16384, /* Max Advertising Interval 500.625ms (801*0.625ms) 16384 */
	NULL); /* Set to NULL for undirected advertising */


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define NOTIFY_INTERVAL 500

static const struct bt_data ad[] =
{
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] =
{
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, SENSOR_HUB_SERVICE_UUID),
};


//BT globals and callbacks
struct bt_conn *m_connection_handle = NULL;
static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_WRN("Connection failed (err %u)\n", err);
		return;
	}

	m_connection_handle = conn;
	LOG_INF("Connected\n");	
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason %u)\n", reason);
	m_connection_handle = NULL;
}

BT_CONN_CB_DEFINE(conn_callbacks) =
{
	.connected        = connected,
	.disconnected     = disconnected,
};


int32_t bt_humidity;
int32_t bt_temperature;

void bt_send_data_thread(void)
{
	while (1) {
        if(m_connection_handle) // if bluetooth connection present
        {
            sensor_hub_update_temperature(m_connection_handle, (uint8_t*)(&bt_temperature), sizeof(bt_temperature));
            sensor_hub_update_humidity(m_connection_handle, (uint8_t*)(&bt_humidity), sizeof(bt_humidity));
        }
		k_sleep(K_MSEC(NOTIFY_INTERVAL));
	}
}

//main is also our sensor sample thread.
int main(void)
{
    int err;

    si7021_init();
    k_msleep(100); // give it some time after resetting.
    
    //Setting up Bluetooth
	err = bt_enable(NULL);
	if(err) 
	{
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
		return -1;
	}
	LOG_INF("Bluetooth initialized");

	if(IS_ENABLED(CONFIG_SETTINGS)) 
	{
		settings_load();
	}

	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if(err) 
	{
		LOG_ERR("Advertising failed to start (err %d)\n", err);
		return -2;
	}
	LOG_INF("Advertising successfully started\n");

	float humidity, temperature_C = 0;
    while(1)
    {    
        humidity = si7021_read_humidity();
        temperature_C = si7021_read_temperature();

        bt_humidity = (int32_t)(humidity);
        bt_temperature = (int32_t)(temperature_C);

        LOG_INF("Humidity: %.2f, Temperature (C) : %.2f", (double)humidity, (double)temperature_C);
        k_msleep(SENSOR_SLEEP_TIME_MS);
    }
	
	return 0;
}

K_THREAD_DEFINE(bt_send_data_thread_id, STACKSIZE, bt_send_data_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
