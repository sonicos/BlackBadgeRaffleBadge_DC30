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
const static char *TAG = "RAFFLE::UI";
static button_t *btn_a, *btn_b;
static int64_t btn_time_pressed;
static int64_t btn_time_released;
static ui_mode_t mode = UI_MODE_BLING;
static ui_mode_t last_mode = UI_MODE_BLING;
static bool btn_a_held = false;
static bool btn_b_held = false;
static bool btn_both_pressed = false;
static bool btn_both_held = false;
static bool btn_both_released = true;
static bool btn_cancel_click = false;
static ui_button_t ui_interaction;
static led_brightness_mode_t temp_brightness;
static bling_state_t *p_state;
static bool ui_enabled = true;


static void __action_adv_master_capture(){
    raffle_adv_temp(RAFFLE_ADV_CAPTURE_M);
}
static void __action_adv_troll_capture(){
    raffle_adv_temp(RAFFLE_ADV_CAPTURE_T);
}
static void __action_capture(){
    raffle_capture_target();
}

static void __ui_brightness()
{
    ESP_LOGD(TAG, "Init Brightness Menu");
    bling_stop();
    last_mode = mode;
    mode = UI_MODE_BRIGHTNESS;
    temp_brightness = p_state->brightness_mode;
    ESP_LOGD(TAG, "Setting brightness from state %d", temp_brightness);
    led_ui_brightness(temp_brightness);
}

static void __ui_brightness_rotate(bool inc)
{
    static int32_t b;
    b = temp_brightness;
    if (inc)
        b++;
    if (!inc)
        b--;
    if (b < 0)
        b = 0;
    if (b > 7)
        b = 7;
    temp_brightness = b;
    led_ui_brightness(b);

    // set all leds to white

    // NON DEV BADGES
    // if zero, do not put an indicator
    // otherwise light up index-1 with red
}

static void __ui_bling()
{
    bling_stop();
    mode = UI_MODE_BLING;
    bling_start();
}
static void __interaction_brightness()
{
    switch (ui_interaction)
    {
    case UI_A_CLICK: // decrement
        __ui_brightness_rotate(false);
        break;
    case UI_B_CLICK: // increment
        __ui_brightness_rotate(true);
        break;
    case UI_A_LONG: // cancel
        bling_set_brightness(p_state->brightness_mode);
        __ui_bling();
        break;
    case UI_B_LONG: // accept
        ESP_LOGD(TAG, "Saving brightness to state %d", temp_brightness);
        p_state->brightness_mode = temp_brightness;
        state_save_indicate();
        __ui_bling();
        break;
    default:
        break;
    }
}
static void __interaction_test()
{
    switch (ui_interaction)
    {
    case UI_A_CLICK: // change direction
        led_main_chase_swap();
        break;
    case UI_B_CLICK: // rotate pulse
        led_eye_pulse_start();
        led_eye_pulse_cycle();
        break;
    case UI_A_LONG: // display flags
        break;
    case UI_B_LONG: // display Config Menu
        break;
    case UI_BOTH_LONG: // restart
        ESP_LOGI(TAG, "Rebooting!");
        esp_restart();
        break;

    case UI_BOTH_EXTENDED: // wipe
        ESP_LOGI(TAG, "Factory Reset");
        unlink(STATE_FILE);
        unlink(PEERS_FILE);
        esp_restart();
        break;

    default:
        break;
    }
}
static void __interaction_bling()
{
    switch (ui_interaction)
    {
    case UI_A_CLICK: // Rotate Bling Pattern
        bling_rotate();

        break;
    case UI_B_CLICK: // rotate bling pallete
        //led_eye_pulse_start();
        //led_eye_pulse_cycle();
        // TODO: rotate color
        break;
    case UI_A_LONG: 
        //TODO: Flags display
        break;
    case UI_B_LONG: // display Config Menu
        __ui_brightness();
        break;
    case UI_BOTH_LONG: 
    #if defined(CONFIG_BADGE_TYPE_MASTER)
        // advertise master capture available
        //__action_adv_master_capture();
    #elif defined(CONFIG_BADGE_TYPE_TROLL) 
        // advertise troll capture available
        //__action_adv_troll_capture(); 
    #else
        __action_capture();
        // TODO: Capture the troll
    #endif
        break;


    case UI_BOTH_EXTENDED: 
    #if defined(CONFIG_BADGE_TYPE_MASTER)
    // Master Menu
    #elif defined(CONFIG_BADGE_TYPE_TROLL)
    // TODO: toggle glitch mode
    #endif
        break;

    default:
        break;
    }
}

// flags menu
static void __interaction_flags()
{
    switch (ui_interaction)
    {
    case UI_BOTH_LONG: // display "unlock screen"
        break;
    default: // return to main menu
        break;
    }
}

static void __interaction_config()
{
    switch (ui_interaction)
    {
    case UI_A_CLICK: // Left
        break;
    case UI_B_CLICK: // Right
        break;
    case UI_A_LONG: // Cancel
        break;
    case UI_B_LONG: // OK
        break;
    default:
        break;
    }
}

// flags menu
static void __interaction_unlock()
{
    switch (ui_interaction)
    {
    case UI_A_LONG: // exit to bling
    case UI_B_LONG:
        break;
    case UI_A_CLICK: // Entry A
        break;
    case UI_B_CLICK: // Entry B
        break;
    case UI_BOTH_LONG: // Entry X
        break;
    default: // return to main menu
        break;
    }
}

static void __interaction()
{
    if (ui_interaction == UI_NOOP)
        return;
    switch (mode)
    {
    case UI_MODE_BLING: // "bling" mode
        __interaction_bling();
        break;
    case UI_MODE_FLAGS: // Flags display
        break;
    case UI_MODE_CONFIG: // Config screen
        break;
    case UI_MODE_BRIGHTNESS: // Config screen
        __interaction_brightness();
        break;
    case UI_MODE_TEST: // Config screen
        __interaction_test();
        break;
    default:
        break;
    }
}
static void __on_ui_btn(button_t *btn, button_state_t state)
{

    if (!ui_enabled) return;
    ui_interaction = UI_NOOP;
    bool btn_a_click = false;
    bool btn_b_click = false;
    bool btn_a_long = false;
    bool btn_b_long = false;
    bool btn_both_long = false;
    bool btn_both_extended = false;

    if (state == BUTTON_PRESSED)
    {
        btn_time_pressed = esp_timer_get_time();
        if (btn_b->internal.state != BUTTON_RELEASED && btn_a->internal.state != BUTTON_RELEASED)
        {
            btn_cancel_click = true;
        }
        btn_both_released = false;
    }
    if (state == BUTTON_RELEASED)
    {

        if (btn_b->internal.state == BUTTON_RELEASED && btn_a->internal.state == BUTTON_RELEASED)
        {
            if (btn_both_held)
            {
                btn_time_released = esp_timer_get_time();
                if (btn_time_released - btn_time_pressed >= UI_HOLD_BOTH_TIME)
                {
                    btn_both_extended = true;
                }
                else
                {
                    btn_both_long = true;
                }

                btn_both_held = false;
                btn_cancel_click = false;
            }
            else if (!btn_cancel_click)
            {
                if (btn == btn_a && btn_a_held)
                {
                    btn_a_held = false;
                    btn_a_long = true;
                }
                else if (btn == btn_b && btn_b_held)
                {
                    btn_b_held = false;
                    btn_b_long = true;
                }
            }
            else if (btn_a_held || btn_b_held)
            {
                // cancel hold activity
                btn_cancel_click = false;
                btn_a_held = false;
                btn_b_held = false;
            }
            btn_both_released = true;
        }
        else
        {
            // btn_both_released= false;
        }
    }
    if (state == BUTTON_PRESSED_LONG)
    {
        if (btn_b->internal.state == BUTTON_PRESSED_LONG && btn_a->internal.state == BUTTON_PRESSED_LONG)
        {
            btn_both_held = true;
            btn_a_held = false;
            btn_b_held = false;
        }
        else if (btn_a->internal.state == BUTTON_PRESSED_LONG)
        {
            btn_both_held = false;
            btn_a_held = true;
            btn_b_held = false;
        }
        else if (btn_b->internal.state == BUTTON_PRESSED_LONG)
        {
            btn_both_held = false;
            btn_a_held = false;
            btn_b_held = true;
        }
        else
        {
            ESP_LOGW(TAG, "BPL - I have NO idea how this happened");
        }
    }

    // Ignore click if both were pressed at some point
    if (btn_cancel_click)
    {
        btn_cancel_click = false;
        // ESP_LOGD(TAG, "Click Cancelled" );
    }
    // normal click
    else if (state == BUTTON_CLICKED && btn_both_released)
    {

        if (btn == btn_a)
        {
            if (btn_b->internal.state == BUTTON_RELEASED)
                btn_a_click = true;
        }
        if (btn == btn_b)
        {
            if (btn_a->internal.state == BUTTON_RELEASED)
                btn_b_click = true;
        }
    }

    if (btn_a_click)
        ui_interaction = UI_A_CLICK;
    if (btn_b_click)
        ui_interaction = UI_B_CLICK;
    if (btn_a_long)
        ui_interaction = UI_A_LONG;
    if (btn_b_long)
        ui_interaction = UI_B_LONG;
    if (btn_both_long)
        ui_interaction = UI_BOTH_LONG;
    if (btn_both_extended)
        ui_interaction = UI_BOTH_EXTENDED;

    // go do the actual operation
    __interaction();
}

void ui_init()
{
    ESP_LOGI(TAG, "Initializing UI");
    btn_a = get_btn_a();
    btn_b = get_btn_b();
    btn_a->callback = __on_ui_btn;
    btn_b->callback = __on_ui_btn;
    p_state = state_bling_get();
    mode = UI_MODE_NOOP;
    last_mode = UI_MODE_NOOP;
}

void ui_enable(){
    ui_enabled = true;
}

void ui_disable(){
    ui_enabled = false;
}

void ui_testmode()
{
    mode = UI_MODE_TEST;
}

void ui_start()
{
    __ui_bling();
}