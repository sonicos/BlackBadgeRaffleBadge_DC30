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

#ifndef COMPONENTS_PEERS_H_
#define COMPONENTS_PEERS_H_
#define PEERS_FILE "/spiflash/peers.dat"
#define PEERS_MAX 100

#define PEERS_COMPANY_ID_BLACKBADGE 0x7777
#define PEERS_COMPANY_ID_ANDNXOR 0x049E
#define PEERS_COMPANY_ID_BLINKYBLING 0x4242
#define PEERS_COMPANY_ID_CPV 0x0C97
#define PEERS_COMPANY_ID_DC503 0x0503
#define PEERS_COMPANY_ID_DC801 0x0801
#define PEERS_COMPANY_ID_DCDARKNET 0x444E
#define PEERS_COMPANY_ID_DCFURS 0x71FF
#define PEERS_COMPANY_ID_DCZIA 0x5050
#define PEERS_COMPANY_ID_GROUND_CONTROL 0x0B25
#define PEERS_COMPANY_ID_JOCO 0x0B25
#define PEERS_COMPANY_ID_TRANS_IONOSPHERIC1 0x0858
#define PEERS_COMPANY_ID_TRANS_IONOSPHERIC2 0x064A

#define PEERS_NVS_KEY "peers"
#define PEERS_NEARBY_TIME_SEC (2 * 60)

typedef struct
{
  uint64_t address;
  uint16_t company_id;
  char name[STATE_NAME_LENGTH + 1];
  uint8_t magic;
  time_t timestamp;
  time_t last_hello;
  uint32_t nonce;
  UT_hash_handle hh;
} peer_t;

typedef enum
{
  BADGE_YEAR_25 = 0x19DC,
  BADGE_YEAR_26 = 0x26DC,
  BADGE_YEAR_27 = 0x27DC,
  BADGE_YEAR_28 = 0x28DC,
  BADGE_YEAR_29 = 0x29DC,
  BADGE_YEAR_30 = 0x30DC,
  NUM_BADGE_YEARS
} BADGE_YEAR;

/**
 * Get a count of the number of peers in the database currently.
 *
 * @return		Number of peers currently being tracked
 */
extern uint32_t peers_count();

/**
 * @brief Dump peers to console
 */
extern void peers_dump();

/**
 * Get a peer by it's 64-bit address
 *
 * @param address	64-bit address of the peer to get
 * @return Peer identified by the address. Or NULL if it does not exist
 */
extern peer_t *peers_get_by_address(uint64_t address);

/**
 * Get the internal hashtable of peers. CAREFUL this is meant for internal
 * purposes
 *
 * @reeturn hashtable of peers
 */
extern peer_t *peers_hashtable_get();

/**
 * Initialize the peers database. This should be done once upon startup.
 * Currently this function does nothing but it is good practice to call it
 * anyways as future implementations may need it.
 *
 * @return		true if successful
 */
extern bool peers_init();

/**
 * Load peers from SD
 */
extern void peers_load();

/**
 * Get a count of peers considered to be nearby
 */
extern uint16_t peers_nearby_count();

/**
 * Save peers to the SD Card
 */
extern void peers_save();

/**
 * Fill peers list with random peers
 */
extern void peers_test_generate();

/**
 * @brief Ensure local peer database is updated with a peer
 * @param other_peer : Peer to add or update the DB with
 * @param hello : Set to true if badge should try to say hello
 * @return The authoritative copy of the peer
 */
extern peer_t *peers_update_or_create(peer_t *other_peer, bool hello);

#endif /* COMPONENTS_PEERS_H_ */
