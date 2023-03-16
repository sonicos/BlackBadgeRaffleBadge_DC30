/*****************************************************************************
 * Made with junk food, tears and sleepless nights in Florida and a Vegas hotel
 *
 * (C) Copyright 2022 Black Badge Raffle Industries, LLC (http://www.blackbadgeraffle.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL JULY 30th, 2022 then,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ADDITIONALLY:
 * If you find this source code useful in anyway, use it in another electronic
 * conference badge (may the odds be ever in your favor), or just think it's neat,
 * ping me on Twitter @DefconRaffle or consider buying me a drink or a badge.
 * I like collecting and tinkering with badges and learing about the process
 * that others undertake to make them.
 * 
 * Most importantly: You must be present to win!
 *
 * Please read the notice file for additional credits
 * 
 *****************************************************************************/

#ifndef COMPONENTS_SYSTEM_H_
#define COMPONENTS_SYSTEM_H_

#include "sdkconfig.h"

// Machine readable version
#define VERSION_INT 100

// Human readable string version
#define __BASE_VERSION__ "1.0.0"
#ifdef CONFIG_BADGE_TYPE_BEACON
#define VERSION __BASE_VERSION__ "B"
#define BADGE_TYPE BADGE_TYPE_BEACON
#elif defined(CONFIG_BADGE_TYPE_TROLL)
#define VERSION __BASE_VERSION__ "T"
#define BADGE_TYPE BADGE_TYPE_TROLL
#elif defined(CONFIG_BADGE_TYPE_MASTER)
#define VERSION __BASE_VERSION__ "M"
#define BADGE_TYPE BADGE_TYPE_MASTER
#elif defined(CONFIG_BADGE_TYPE_ENTRANT)
#define VERSION __BASE_VERSION__ "E"
#define BADGE_TYPE BADGE_TYPE_ENTRANT
#else
#define VERSION __BASE_VERSION__ ""
#define BADGE_TYPE BADGE_TYPE_NORMAL
#endif

#ifndef APP_CPU_NUM
#define APP_CPU_NUM 0
#endif

// Standard Includes
#include <byteswap.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
//#include <sys/unistd.h>

// ESP Includes

#include "esp_system.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_adc_cal.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_gap_ble_api.h"
#include "esp_heap_caps.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_vfs_dev.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "driver/usb_serial_jtag.h"
#include "esp_vfs_usb_serial_jtag.h"
#include "esp_task_wdt.h"
#include "led_strip.h"
#include "nvs_flash.h"
#include "linenoise/linenoise.h"
#include "lwip/netdb.h"
#include "lwip/apps/sntp.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"
#include "rom/crc.h"
#include "esp_netif.h"
#include "esp_sntp.h"
//#include "argtable3/argtable3.h"

#include "esp_idf_lib_helpers.h"
#include "button.h"
// 3rd party
//#include "cJSON.h"
//#include "esp_request.h"
#include "lib8tion.h"
#include "uthash.h"
#include "tpl.h"
#include "rgb.h"

// Black Badge Raffle Includes
#include "util.h"
#include "drawing.h"
#include "raffle.h"
#include "ble.h"
#include "boot.h"
#include "btn.h"
//#include "botnet/botnet.h"
#include "console.h"
//#include "chip8.h"
//#include "drv_config.h"
//#include "drv_greenpak.h"
//#include "drv_ili9225.h"
//#include "drv_is31fl.h"
//#include "drivers/drv_lis2de12.h"
//#include "drv_sd.h"
//#include "btn.h"
//#include "gfx.h"
//#include "botnet/botnet_ui.h"
//#include "hal_i2c.h"
//#include "addons.h"
#include "led.h"
#include "mesh.h"
#include "reboot.h"
//#include "lolcode.h"
//#include "ota.h"
//#include "post.h"
//#include "skifree.h"
#include "state.h"
#include "peers.h"
#include "sd.h"
//#include "qdbmp.h"
#include "time_manager.h"
#include "ui.h"
#include "unlocks.h"
//#include "ymodem.h"
#include "wifi.h"

#define INPUT_CHAR_MIN 32
#define INPUT_CHAR_MAX 126

#define TASK_PRIORITY_LOW 2
#define TASK_PRIORITY_BLE 4
#define TASK_PRIORITY_CONSOLE 3
#define TASK_PRIORITY_PLAY_BLING 3
#define TASK_PRIORITY_ADDONS 4
#define TASK_PRIORITY_MEDIUM 5
#define TASK_PRIORITY_STATE_SAVE 1
#define TASK_PRIORITY_HIGH 8

#define BBR_NVS_NAMESPACE "RAFFLE"

#define STRING_ENCRYPTION_KEY "0000000000000000"

#endif /* COMPONENTS_SYSTEM_H_ */