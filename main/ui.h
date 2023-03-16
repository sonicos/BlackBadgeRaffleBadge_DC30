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

#ifndef COMPONENTS_UI_H_
#define COMPONENTS_UI_H_

#define UI_HOLD_BOTH_TIME 5000 * 1000 // ms * ticks

typedef enum
{
    UI_MODE_BLING,
    UI_MODE_FLAGS,
    UI_MODE_CONFIG,
    UI_MODE_TROLL,
    UI_MODE_MASTER,
    UI_MODE_TEST,
    UI_MODE_BRIGHTNESS,
    UI_MODE_NOOP,
} ui_mode_t;
typedef enum
{
    UI_NOOP = 0,
    UI_A_CLICK,
    UI_B_CLICK,
    UI_A_LONG,
    UI_B_LONG,
    UI_BOTH_LONG,
    UI_BOTH_EXTENDED,
} ui_button_t;
extern void ui_init();
extern void ui_testmode();
extern void ui_start();
extern void ui_enable();
extern void ui_disable();

#endif