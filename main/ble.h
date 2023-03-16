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

#ifndef COMPONENTS_BLE_H_
#define COMPONENTS_BLE_H_

// Appearance values to use for rotating BLE GAP. These are in LSB so on the
// wire they look normal
#define APPEARANCE_NONE 0x00
#define APPEARANCE_BADGELIFE 0x30DC /* Badgelife */
#define APPEARANCE_BROADCAST 0x01C0 /* Eyeglasses LOL */
#define APPEARANCE_BOTNET 0x0080    /* Computer */
#define APPEARANCE_TIME 0x0010      /* Clock */
#define APPEARANCE_RAFFLE 0x7707    /* Raffle */
#define APPEARANCE_MESH 0xEDFE      /* Mesh State */

// Appearance values for other badges
#define APPEARANCE_BADGELIFE_DC29 0x29DC
#define APPEARANCE_BADGELIFE_DC27 0x27DC
#define APPEARANCE_BADGELIFE_DC26 0x26DC
#define APPEARANCE_BADGELIFE_DC25 0x19DC
#define APPEARANCE_PI 0x3713 /* pi */

#define BLE_MANUFACTURER_ID 0x7777
#define BLE_BROADCAST_MAX_LEN 16
#define BLE_TIME_CANARY 0x4F
#define BLE_MAGIC_RABIES 0x35

#define DATA_LENGTH_DC25 12
#define DATA_LENGTH_ALT_BEACON 26

#define BEACON_A_SERIAL 0xa1dae8c4 // Scav
#define BEACON_B_SERIAL 0xa1dae4e0 // Packet Hacking
#define BEACON_C_SERIAL 0xa1dae5a0 // SE Village
#define BEACON_D_SERIAL 0xf983b8c8 // Crypto
#define BEACON_E_SERIAL 0xa1dae6e8 // Hardware Hacking

#ifdef CONFIG_BADGE_TYPE_BEACON
#define BLE_POWER_LEVEL ESP_PWR_LVL_P9
#else
#define BLE_POWER_LEVEL ESP_PWR_LVL_P6
//#define BLE_POWER_LEVEL ESP_PWR_LVL_N0
#endif

#define BEACON_1_UUID                                                       \
  {                                                                         \
    0x4f, 0x96, 0xc4, 0xcc, 0xe8, 0x22, 0x43, 0x92, 0xae, 0x8d, 0x2f, 0x31, \
        0x8c, 0xb8, 0x7e, 0x7c                                              \
  }
#define BEACON_2_UUID                                                       \
  {                                                                         \
    0x5f, 0xce, 0x06, 0x81, 0xfd, 0x78, 0x49, 0x61, 0xa4, 0x12, 0x81, 0x3a, \
        0xe3, 0x9b, 0x6f, 0x3a                                              \
  }
#define BEACON_3_UUID                                                       \
  {                                                                         \
    0x7f, 0xc4, 0x2f, 0xce, 0xe0, 0x97, 0x49, 0x29, 0xaf, 0xc3, 0x23, 0xdc, \
        0xfc, 0x41, 0xe7, 0xfd                                              \
  }

#define BEACON_UUID_LENGTH 16
#define BEACON_PREAMBLE    \
  {                        \
    0x18, 0x01, 0xBE, 0xAC \
  }
#define BEACON_PREAMBLE_LENGTH 4

typedef struct
{
  uint16_t company_id;
  // uint8_t magic;
  uint16_t flags;
  uint32_t serial;

  uint8_t nonce[4];
} ble_advertisement_general_t;

// typedef struct {
//   uint16_t company_id;
//   uint8_t seconds[4];
//   uint8_t useconds[4];
//   uint8_t stratum;
//   uint8_t version[2];
//   uint8_t padding[1];
//   uint8_t crc[4];
//   uint8_t nonce[4];
// } ble_advertisement_pi_t;

typedef struct
{
  uint8_t seconds[sizeof(time_t)];
  uint8_t useconds[sizeof(suseconds_t)];
  uint8_t stratum;
  uint8_t canary;
  uint8_t padding[4]; /* For AES ECB purposes */
  uint8_t crc[2];     /* Must be last */
} ble_advertisement_time_t;

// Enumeration used for round robining BLE GAPs
typedef enum
{
  ble_rotate_general,
  ble_rotate_raffle,
  ble_rotate_mesh_state,
  ble_rotate_time,
  ble_rotate_badgelife_mimic,
  ble_rotate_count // Counter of possible rotation modes
} ble_rotation_mode_t;

// Enumeration for all possible special BLE modes
typedef enum
{
  special_ble_mode_none,
  special_ble_mode_badgelife_mimic
} special_ble_mode_t;

extern void ble_special_mode_set(special_ble_mode_t mode);
extern void ble_init();

extern void ble_unlock_update();
extern void ble_name_set(const char *name);

/**
 * Starts the nonce rotation task
 */
extern void ble_nonce_task_start();
extern void ble_rotate_task_start();
extern void ble_begin_processing();

#endif /* COMPONENTS_BLE_H_ */
