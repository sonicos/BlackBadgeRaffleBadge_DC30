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

// uncomment the ifdef when making a master, otherwise it needs to be commented out.
//#ifdef CONFIG_BADGE_TYPE_MASTER
#include "system.h"

#ifndef COMPONENTS_BBDRAWING_H_
#define COMPONENTS_BBDRAWING_H_

#define DRAWING_FILE "/spiflash/drawing.dat"
#define DRAWING_MAX 200

#define DRAWING_NEARBY_TIME_SEC (5 * 60)

typedef struct
{
  //uint64_t address;
  uint32_t serial;
  uint8_t badge_type;
  uint16_t unlock_flags;
  time_t timestamp;
  UT_hash_handle hh;
} drawing_t;




/**
 * Get a count of the number of drawings in the database currently.
 *
 * @return		Number of drawings currently being tracked
 */
extern uint32_t drawings_count();

/**
 * @brief Dump drawings to console
 */
extern void drawings_dump();

/**
 * Get a drawing by it's 64-bit address
 *
 * @param address	64-bit address of the drawing to get
 * @return drawing identified by the address. Or NULL if it does not exist
 */
extern drawing_t *drawings_get_by_address(uint32_t serial);

/**
 * Get the internal hashtable of drawings. CAREFUL this is meant for internal
 * purposes
 *
 * @reeturn hashtable of drawings
 */
extern drawing_t *drawings_hashtable_get();

/**
 * Initialize the drawings database. This should be done once upon startup.
 * Currently this function does nothing but it is good practice to call it
 * anyways as future implementations may need it.
 *
 * @return		true if successful
 */
extern bool drawings_init();

/**
 * Load drawings from SD
 */
extern void drawings_load();

/**
 * Get a count of drawings considered to be nearby
 */
extern uint16_t drawings_nearby_count();

/**
 * Save drawings to the SD Card
 */
extern void drawings_save();

extern void drawing_start();
extern void drawing_select_winner();

/**
 * Fill drawings list with random drawings
 */
extern void drawings_test_generate();

/**
 * @brief Ensure local drawing database is updated with a drawing
 * @param other_drawing : drawing to add or update the DB with
 * @param hello : Set to true if badge should try to say hello
 * @return The authoritative copy of the drawing
 */
extern drawing_t *drawings_update_or_create(drawing_t *other_drawing);
extern void drawings_mutex_lock();
extern void drawings_mutex_unlock();
extern void drawings_process(drawing_t drawing);


#endif

// uncomment the next endif when making a master, otherwise it needs to be commented out.
//#endif