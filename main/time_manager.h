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

#ifndef COMPONENTS_TIME_MANAGER_H_
#define COMPONENTS_TIME_MANAGER_H_

#define TIME_MAX_STRATUM 16
#define TIME_DEFAULT 1659139200    // Saturday, July 30, 2022 12:00:00 AM
#define BEER_TIME_1_UTC 1660543200 // Afterparty! Monday, August 15, 2022 6:00:00 AM
#define BEER_TIME_2_UTC 1534042800

// extern void time_manager_start();

/**
 * @brief Handle a BLE advertised time
 */
extern void time_manager_advertisement_handle(
    ble_advertisement_time_t ble_time);

/**
 * @brief Start up background task that checks for beer times
 */
extern void time_manager_beer_time_task_start();

/**
 * @brief Get the current time stamp in seconds
 */
extern uint32_t time_manager_now_sec();

/**
 * @brief Set the current time of day seconds and microseconds
 * @param sec : Time now in seconds since epoc
 * @param usec : Time now micro seconds
 * @param source_stratum : The stratum of the time source
 */
extern void time_manager_set_time(uint32_t sec,
                                  uint32_t usec,
                                  uint8_t source_stratum);

/**
 * @brief Get the current stratum this badge is advertising time as
 */
extern uint8_t time_manager_stratum_get();

/**
 * @brief Set the current time stratum for the badge
 *
 * @param stratum Stratum to store
 */
extern void time_manager_stratum_set(uint8_t stratum);

/**
 * @brief Attempt to sync with a time source and stratum
 * @param seconds : Time now in seconds since epoch
 * @param useconds : Micro seconds
 * @param stratum : Stratum of the time source
 */
extern void time_manager_sync(uint32_t seconds,
                              uint32_t useconds,
                              uint8_t stratum);
extern void time_manager_ntpupdate(struct timeval *time);

#endif /* COMPONENTS_TIME_MANAGER_H_ */
