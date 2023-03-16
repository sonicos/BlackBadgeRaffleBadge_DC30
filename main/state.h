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

#include "system.h"

#ifndef COMPONENTS_STATE_H_
#define COMPONENTS_STATE_H_

#define STATE_FILE "/spiflash/badge.bin"

#define STATE_BRIGHTNESS_MIN 1
#define STATE_BRIGHTNESS_MAX 5
#define STATE_BRIGHTNESS_DEFAULT 3

#define STATE_NAME_LENGTH 8
#define STATE_NAME_DEFAULT "RAFFLE"

#define STATE_BADGELIFE_MAGIC_BYTE_DEFAULT 0

#define STATE_TILT_ENABLED_DEFAULT true

#define STATE_UNLOCK_DEFAULT 0

typedef struct
{
  char name[STATE_NAME_LENGTH + 1]; //+1 for null termination
  uint32_t time_sec;
  bool airplane_mode_enabled;
  uint16_t unlock_state;
  // botnet_state_t botnet_state; //10
  bling_state_t bling_state;
  uint8_t badgelife_magic_byte;
  game_data_t console_state; // 412
  mesh_state_t mesh_state;   // 10?
  uint8_t adv_flags;
  uint8_t padding[3];        // make sure it's aligned on 128-bit boundary
  uint32_t crc;
} badge_state_t; // 464

/**
 * Initilize badge state handler. This does not load any data, merely gets
 * things ready
 */
extern void state_init();
// extern botnet_state_t* state_botnet_get();
extern bling_state_t *state_bling_get();
// extern game_data_t* state_console_get();

extern mesh_state_t *state_mesh_get();
extern uint8_t *state_adv_flags_get();
void state_unlock(uint16_t unlock);

/**
 * Load the badge state from NVS storage.
 *
 * NOTE: This should be done from the main task or a task with at least low
 * priority (not idle)
 */
extern void state_load();

/**
 * @brief Get the current badgelife magic byte
 */
extern uint8_t state_badgelife_magic_byte_get();

/**
 * @brief Set the badgelife magic byte
 */
extern void state_badgelife_magic_byte_set(uint8_t magic_byte);
// extern uint8_t state_brightness_get();
// extern void state_brightness_set(uint8_t brightness);
extern bool state_hide_bling_get();
extern void state_hide_bling_set(bool hide);
extern void state_name_get(char *name);
extern void state_name_set(char *name);
void state_raffle_adv_flag_remove(raffle_adv_flags_t flag);
void state_raffle_adv_flag_add(raffle_adv_flags_t flag);

/**
 * @brief Reset badge state to new
 */
extern void state_new();

/**
 * @brief Indicate to badge that something has changed in state and it needs
 * saving
 */
extern void state_save_indicate();

/**
 * @brief Run a task in the background to handle saving state (also saves peers)
 */
extern void state_save_task_start();

/**
 * @brief Get the current high pong score
 */
extern uint32_t state_score_pong_get();

/**
 * @brief Set the current high pong score
 */
extern void state_score_pong_set(uint32_t score);

/**
 * @brief Get the current high ski score
 */
extern uint32_t state_score_ski_get();

/**
 * Set the curren high ski free score
 */
extern void state_score_ski_set(uint32_t score);

/**
 * @brief Get the current tilt enabled state
 */
extern bool state_tilt_get();

/**
 * @brief Enable or disable the screen tilt
 */
extern void state_tilt_set(bool tilt);

/**
 * @brief Get the current unlock state
 * @return Current unlock state as a bitmask
 */
extern uint16_t state_unlock_get();

/**
 * @brief Set the current unlock state
 * @param unlock : The unlock state as a bitmask
 */
extern void state_unlock_set(uint16_t unlock);

extern bool state_is_unlocked(uint16_t unlock);

#endif
