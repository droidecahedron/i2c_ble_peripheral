# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(i2c_ble_peripheral)

zephyr_include_directories(.) # (inc)

# NORDIC SDK APP START
target_sources(app PRIVATE
	src/main.c
	src/si7021.c
	src/services/ble_shs.c
)

# NORDIC SDK APP END
