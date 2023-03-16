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

#ifndef COMPONENTS_REBOOT_H_
#define COMPONENTS_REBOOT_H_

#define REBOOT_TIME_LIMIT 45 * 60 * 1000
#define REBOOT_TIME_MASTER_IGNORE 10 * 60 * 1000
#define REBOOT_TIME_INCREMENT 17 * 1000 // Avoid multiples of 30 seconds

typedef enum
{
    reboot_menu,
    reboot_mesh,
    reboot_save,
    reboot_console
} reboot_mode_t;

extern void reboot_prevent(reboot_mode_t mode);
extern void reboot_enable(reboot_mode_t mode);
extern void reboot_master_override();
extern void reboot_terrible_hack_timer_start();
extern void reboot_trigger();
#endif