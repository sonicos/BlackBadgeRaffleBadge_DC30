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

#ifndef COMPONENTS_UNLOCKS_H_
#define COMPONENTS_UNLOCKS_H_

#define UNLOCK_CONSOLE      1 << 0 // enter friend at prompt
//#define UNLOCK_GOAT         1 << 1 //
#define UNLOCK_CODE_B       1 << 2 // Badge QR
#define UNLOCK_TROLL_INNOC  1 << 3 // Captured a Troll
#define UNLOCK_MASTER_INNOC 1 << 4 // Master capture
#define UNLOCK_ATEAM        1 << 5 
#define UNLOCK_PARTY_TIME   1 << 6
#define UNLOCK_CODE_MUD     1 << 7  // Heartbeat
#define UNLOCK_BEACON_A     1 << 8  // Scav
#define UNLOCK_BEACON_B     1 << 9  // Packet Hacking
#define UNLOCK_BEACON_C     1 << 10 // SE Village
#define UNLOCK_BEACON_D     1 << 11 // Crypto
#define UNLOCK_BEACON_E     1 << 12 // Hardware Hacking
#define UNLOCK_UNUSED_1     1 << 13
#define UNLOCK_UNUSED_2     1 << 14
#define UNLOCK_WINNER       1 << 15

// #define BEACON_A_SERIAL 0xa1dae8c4; // Scav
// #define BEACON_B_SERIAL 0xa1dae4e0; // Packet Hacking
// #define BEACON_C_SERIAL 0xa1dae5a0; // SE Village
// #define BEACON_D_SERIAL 0xf983b8c8; // Crypto
// #define BEACON_E_SERIAL 0xa1dae6e8; // Hardware Hacking

#define UNLOCK_PASSWORD_A = "ABBAXAAA"; // GOAT
#define UNLOCK_PASSWORD_B = "XBBABXBB";

#endif /* COMPONENTS_UNLOCKS_H_ */
