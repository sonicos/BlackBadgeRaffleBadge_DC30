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

const static char *TAG = "RAFFLE::Btn";

static const char *states[] = {
    [BUTTON_PRESSED] = "pressed",
    [BUTTON_RELEASED] = "released",
    [BUTTON_CLICKED] = "clicked",
    [BUTTON_PRESSED_LONG] = "pressed long",
};

static button_t btn1, btn2;
static bool btn1_pressed = false;
static bool btn2_pressed = false;

static void on_button_test(button_t *btn, button_state_t state)
{
    ESP_LOGI(TAG, "%s button %s", btn == &btn1 ? "First" : "Second", states[state]);
    switch (state)
    {
    case BUTTON_CLICKED:
        if (btn == &btn1)
            led_main_chase_swap();
        if (btn == &btn2)
            led_eye_pulse_cycle();
        break;
    default:
        break;
    }
}

static void __init_function(button_t *btn, button_state_t state)
{
    ESP_LOGI(TAG, "INIT - %s button %s", btn == &btn1 ? "First" : "Second", states[state]);
    switch (state)
    {
    case BUTTON_PRESSED:
        if (btn == &btn1)
            btn1_pressed = true;
        if (btn == &btn2)
            btn2_pressed = true;
        break;
    case BUTTON_RELEASED:
        if (btn == &btn1)
            btn1_pressed = false;
        if (btn == &btn2)
            btn2_pressed = false;
        break;
    default:
        break;
    }
}

void buttons_init()
{
    ESP_LOGI(TAG, "Initializing Buttons");
    btn1.gpio = GPIO_BTN_A;
    btn1.pressed_level = 0;
    btn1.internal_pull = true;
    btn1.autorepeat = false;

    btn2.gpio = GPIO_BTN_B;
    btn2.pressed_level = 0;
    btn2.internal_pull = true;
    btn2.autorepeat = false;
    // wait a to let levels get situated
    btn1.callback = __init_function;
    btn2.callback = __init_function;
    ESP_ERROR_CHECK(button_init(&btn1));
    ESP_ERROR_CHECK(button_init(&btn2));
    DELAY(50);
}

bool button_boot_check()
{
    ESP_LOGI(TAG, "Checking button state");

    if (btn1_pressed && btn2_pressed)
        return true;
    return false;
}

void btn_test_mode()
{

    btn1.callback = on_button_test;
    btn2.callback = on_button_test;

    // ESP_ERROR_CHECK(button_init(&btn1));
    // ESP_ERROR_CHECK(button_init(&btn2));
}

void btn_set_callback(button_event_cb_t callback)
{
    btn1.callback = callback;
    btn2.callback = callback;
}

button_t *get_btn_a()
{
    return &btn1;
}

button_t *get_btn_b()
{
    return &btn2;
}