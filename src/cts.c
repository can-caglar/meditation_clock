/** @file
 *  @brief CTS Service sample
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/drivers/rtc.h>

static uint8_t ct[10];

static void set_current_time(uint8_t *buf);
static void get_current_time(uint8_t *buf);

static const struct device* rtc_pcf8563 = DEVICE_DT_GET(DT_NODELABEL(pcf8563t));

static void ct_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	/* TODO: Handle value */
}

static ssize_t read_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
		       void *buf, uint16_t len, uint16_t offset)
{
	uint8_t *value = attr->user_data;

	get_current_time(value);

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(ct));
}

static ssize_t write_ct(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			const void *buf, uint16_t len, uint16_t offset,
			uint8_t flags)
{
	uint8_t *value = attr->user_data;

	if (offset + len > sizeof(ct)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	memcpy(value + offset, buf, len);
	
	set_current_time(value);

	return len;
}

/* Current Time Service Declaration */
BT_GATT_SERVICE_DEFINE(cts_cvs,
	BT_GATT_PRIMARY_SERVICE(BT_UUID_CTS),
	BT_GATT_CHARACTERISTIC(BT_UUID_CTS_CURRENT_TIME, BT_GATT_CHRC_READ |
			       BT_GATT_CHRC_NOTIFY | BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
			       read_ct, write_ct, ct),
	BT_GATT_CCC(ct_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

void get_current_time(uint8_t *buf)
{
	uint16_t year;

	/* 'Exact Time 256' contains 'Day Date Time' which contains
	 * 'Date Time' - characteristic contains fields for:
	 * year, month, day, hours, minutes and seconds.
	 */

	struct rtc_time time = { 0 };
	rtc_get_time(rtc_pcf8563, &time);

	year = sys_cpu_to_le16(1900 + time.tm_year);
	memcpy(buf,  &year, 2); /* year */
	buf[2] = time.tm_mon + 1; /* months starting from 1 */
	buf[3] = time.tm_mday; /* day */
	buf[4] = time.tm_hour; /* hours */
	buf[5] = time.tm_min; /* minutes */
	buf[6] = time.tm_sec; /* seconds */

	/* 'Day of Week' part of 'Day Date Time' */
	buf[7] = time.tm_wday + 1; /* day of week starting from 1 */

	/* 'Fractions 256 part of 'Exact Time 256' */
	buf[8] = 0U;

	/* Adjust reason */
	buf[9] = 0U; /* No update, change, etc */
}

static void set_current_time(uint8_t *buf)
{
	uint16_t year = sys_le16_to_cpu((uint16_t)buf[0] | (buf[1] << 8)) - 1900;

	struct rtc_time t = {
		.tm_year = year,
		.tm_mon = buf[2] - 1,
		.tm_mday = buf[3],
		.tm_hour = buf[4],
		.tm_min = buf[5],
		.tm_sec = buf[6],
	};
	rtc_set_time(rtc_pcf8563, &t);
}

void cts_notify(void)
{	
	get_current_time(ct);
	bt_gatt_notify(NULL, &cts_cvs.attrs[1], &ct, sizeof(ct));
}
