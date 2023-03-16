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

#ifndef COMPONENTS_MESH_H_
#define COMPONENTS_MESH_H_



typedef struct
{
    uint32_t winner_serial;
    uint32_t timestamp_seconds;
    uint8_t mesh_flags;
    uint8_t stratum;
} mesh_state_t;

typedef enum
{
    MESH_RAFFLE_STARTED = 1 << 0,
    MESH_RAFFLE_ENDED = 1 << 1,
    MESH_BEACONS_DISABLED = 1 << 2,
    MESH_TROLL_GLITCH_DISABLED = 1 << 3,
} mesh_flags_t;

typedef struct
{
    uint32_t winner_serial;
    uint32_t timestamp_seconds;
    uint8_t mesh_flags;
    uint8_t stratum;
    uint8_t padding[4];
    uint8_t crc[2]; // This must be last
} ble_advertisement_mesh_t;

extern void mesh_init();
extern void mesh_state_reset();
extern ble_advertisement_mesh_t *mesh_packet_get();
extern void mesh_advertiser_handler(ble_advertisement_mesh_t *packet);
extern void bbrmesh_mutex_lock();
extern void bbrmesh_mutex_unlock();

#ifdef CONFIG_BADGE_TYPE_MASTER
extern void mesh_flag_add(mesh_flags_t flag);
extern void mesh_flag_remove(mesh_flags_t flag);
extern void mesh_set_winner(uint32_t serial);
extern void mesh_master_reset();
#endif
#endif