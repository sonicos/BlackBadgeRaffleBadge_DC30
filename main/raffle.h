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

#pragma once
#include "system.h"

#ifndef COMPONENTS_BOTNET_BOTNET_H_
#define COMPONENTS_BOTNET_BOTNET_H_

// Must be 16
typedef struct
{
  uint16_t unlock_flags;
  uint8_t badge_type;
  uint8_t adv_flags;
  uint32_t serial;
  uint16_t random;
  uint8_t padding[4];
  uint8_t crc[2]; // This must be last
} ble_advertisement_raffle_t;

typedef enum
{
  BADGE_TYPE_NORMAL = 0,
  BADGE_TYPE_ENTRANT,
  BADGE_TYPE_TROLL,
  BADGE_TYPE_MASTER,
  BADGE_TYPE_WINNER,
  BADGE_TYPE_BEACON,
  BADGE_TYPE_COUNT
} badge_type_t;

typedef enum
{
  RAFFLE_ADV_GLITCH = 1 << 0, 
  RAFFLE_ADV_CAPTURE_T = 1 << 1,
  RAFFLE_ADV_CAPTURE_M = 1 << 3,
  RAFFLE_ADV_REBOOT = 1 << 3,
  RAFFLE_ADV_GIFT = 1 << 4,
  RAFFLE_ADV_GIFT_M = 1 << 5,
  RAFFLE_ADV_RES1 = 1 << 6,
  RAFFLE_ADV_RES2 = 1 << 7,
} raffle_adv_flags_t;

typedef enum {
  RAFFLE_CAPTURE_NONE,
  RAFFLE_CAPTURE_TROLL,
  RAFFLE_CAPTURE_MASTER,

} raffle_capture_target_t;

void raffle_adv_flag_add(raffle_adv_flags_t flag);

void raffle_adv_flag_remove(raffle_adv_flags_t flag);

void set_capture_target(raffle_capture_target_t target);

/**
 * Handle botnet advertisements individually
 *
 * @param packet	Pointer to packet that we need to parse/handle
 */
// extern void botnet_advertiser_handler(ble_advertisement_botnet_t* packet);

extern void raffle_advertiser_handler(ble_advertisement_raffle_t *packet);

extern void raffle_unlock_update();

/**
 * Get a pointer to the current botnet packet to advertise
 *
 * @return a pointer to the current botnet packet
 */
extern ble_advertisement_raffle_t *raffle_packet_get();

/**
 * @brief Initialize the botnet
 */
extern void raffle_init();

/**
 * @brief Level up the player
 */
extern void botnet_level_up();

/**
 * @brief : Set a botnet packet. This includes setting the CRC
 * @param p_packet : Pointer to botnet packet to set
 */
extern void raffle_packet_set(ble_advertisement_raffle_t *p_packet);

/**
 * @brief Validate a given botnet packet
 * @param p_packet : Pointer to the packet to validate
 * @return True if valid
 */
extern bool raffle_payload_validate(ble_advertisement_raffle_t *p_packet);

/**
 * @brief Reset botnet state, careful player will lose all progress!
 */
extern void botnet_reset();

extern void raffle_adv_temp(uint8_t flags);
extern void raffle_capture_target();

#endif /* COMPONENTS_BOTNET_BOTNET_H_ */
