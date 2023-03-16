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

#ifndef COMPONENTS_BTN_H_
#define COMPONENTS_BTN_H_

#ifdef BOARD_TYPE_DEV
#define GPIO_BTN_A 18
#define GPIO_BTN_B 19
#else
#define GPIO_BTN_A 6
#define GPIO_BTN_B 7
#endif

// enum buttons {btn_a, btn_b};

extern void buttons_init();
extern void btn_test_mode();
extern bool button_boot_check();
extern void btn_set_callback(button_event_cb_t callback);
extern button_t *get_btn_a();
extern button_t *get_btn_b();
#endif /* COMPONENTS_BTN_H_ */