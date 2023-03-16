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

// Define some colors

// tasksen

// eye color
const static char *TAG = "RAFFLE::LED";
static bling_state_t *p_state;

static volatile uint16_t m_led_main_blink_delay = 1000;
static volatile uint16_t m_led_main_chase_delay = 125;
static uint8_t m_brightness = 10; // this is what we're dividing by
static uint8_t s_led_state = 0;
static led_strip_t *pLedMain;
static led_strip_t *pLedEye;
static bool m_led_main_blink_run = false;
static bool m_led_main_chase_run = false;
static bool m_led_eye_pulse_run = false;

// defaults that we shouldn't need to use
volatile static int16_t led_eye_pulse_max_intensity = 70;
volatile static int16_t led_eye_pulse_delay = 50;
volatile static eye_color_t led_eye_pulse_color = EYE_RED;
volatile static led_color_t m_led_main_chase_color = EYE_RED;
static bool led_main_chase_forward = true;

volatile static bool bling_interrupted = false;
volatile static bool bling_priority_interrupted = false;
volatile static bool bling_enabled = true;
volatile static bool bling_run = false;

volatile static bool m_glitch_run = false;
volatile static bool m_glimmer_run = false;
volatile static bool m_raffle_run = false;
volatile static bool m_winner_run = false;
volatile static bool m_loser_run = false;
volatile static bool m_starfield_run = false;
volatile static bool m_entrant_run = false;
volatile static bool m_official_run = false;


static SemaphoreHandle_t m_mutex = NULL;
static StaticSemaphore_t m_mutex_buffer;
// pthread_mutex_t mtx_led_eye=PTHREAD_MUTEX_INITIALIZER;
static rgb_t COLOR_GOLD = { .r = 255, .g= 215, .b =0};
static rgb_t COLOR_GREEN = { .r = 0, .g= 255, .b =0};
static rgb_t COLOR_RED = { .r = 255, .g= 0, .b =0};
static rgb_t COLOR_BLUE = { .r = 0, .g= 0, .b =255};
// TODO: Rainbow rotator

// todo: function to set pulse color
// color object

void led_mutex_lock(){
  xSemaphoreTake(m_mutex,portMAX_DELAY);
}
void led_mutex_unlock(){
  xSemaphoreGive(m_mutex);
}

void __bling_blink_task(void *params)
{
    ESP_LOGD(TAG, "Starting Main LED task");
    m_led_main_blink_run = true;
    while (m_led_main_blink_run)
    {
        // ESP_LOGI(TAG, "Blink!");
        // pLedMain->set_pixel(pLedMain, 0, 2,  0, 0);
        pLedMain->set_pixel(pLedMain, 0, 4, 0, 0);   // 64
        pLedMain->set_pixel(pLedMain, 1, 8, 0, 0);   // 32
        pLedMain->set_pixel(pLedMain, 2, 16, 0, 0);  // 16
        pLedMain->set_pixel(pLedMain, 3, 32, 0, 0);  // /8
        pLedMain->set_pixel(pLedMain, 4, 64, 0, 0);  // /4
        pLedMain->set_pixel(pLedMain, 5, 128, 0, 0); // /2
        pLedMain->set_pixel(pLedMain, 6, 255, 0, 0); // /1
        pLedMain->refresh(pLedMain, 100);
        DELAY(m_led_main_blink_delay);
        pLedMain->clear(pLedMain, 50);
        DELAY(m_led_main_blink_delay);
    }

    vTaskDelete(NULL);
}

void __bling_starfield_task(void *params)
{
    ESP_LOGD(TAG, "Starting Main LED task");
    m_starfield_run = true;
    int max_bright = 255/m_brightness;
    static rgb_t color[7];
    static rgb_t adj_color[7];
    uint16_t brightness = 0;
    static uint8_t offset[7];
    int i;
    for (i=0; i<7; i++){
        color[i].r = esp_random()%255;
        color[i].g = esp_random()%255;
        color[i].b = esp_random()%255;
        offset[i] = esp_random()%20;
    }
    while (m_starfield_run)
    {
        // ESP_LOGI(TAG, "Blink!");
        // pLedMain->set_pixel(pLedMain, 0, 2,  0, 0);
        //uint16_t l_bright = brightness%max_bright;
        for (i=0; i<7; i++){
            int m_bright = (brightness + offset[i])%max_bright;
            if (m_bright == 0) {
                color[i].r = esp_random()%255;
                color[i].g = esp_random()%255;
                color[i].b = esp_random()%255;
                offset[i] = esp_random()%10;
            } else {
                adj_color[i] = rgb_scale(color[i],m_bright);
                pLedMain->set_pixel(pLedMain, i, adj_color[i].r, adj_color[i].g, adj_color[i].b);   // 64
            }
            // color[i].g = esp_random()%255;
            // color[i].b = esp_random()%255;
            // offset[i] = esp_random()%8
        }
        pLedMain->refresh(pLedMain, 100);
        DELAY(175);
        brightness++;
        if(brightness > 65000) brightness =0;
        // pLedMain->set_pixel(pLedMain, 0, 4, 0, 0);   // 64
        // pLedMain->set_pixel(pLedMain, 1, 8, 0, 0);   // 32
        // pLedMain->set_pixel(pLedMain, 2, 16, 0, 0);  // 16
        // pLedMain->set_pixel(pLedMain, 3, 32, 0, 0);  // /8
        // pLedMain->set_pixel(pLedMain, 4, 64, 0, 0);  // /4
        // pLedMain->set_pixel(pLedMain, 5, 128, 0, 0); // /2
        // pLedMain->set_pixel(pLedMain, 6, 255, 0, 0); // /1
        // pLedMain->refresh(pLedMain, 100);
        // DELAY(m_led_main_blink_delay);
        // pLedMain->clear(pLedMain, 50);
        // DELAY(m_led_main_blink_delay);
    }
    pLedMain->clear(pLedMain, 50);
    vTaskDelete(NULL);
}

void __bling_winner_task(void *params){
ESP_LOGD(TAG, "Starting winner LED task");
    m_winner_run = true;
    int8_t max_brightness = 255/m_brightness;
    int16_t brightness = 0;
    int8_t delta = 1;
    uint16_t _delay = 2000/max_brightness;
    rgb_t color_a = COLOR_GOLD;
    rgb_t color_b = COLOR_GREEN;
    while (m_winner_run)
    {
        
        if (brightness==0){
            max_brightness = 255/m_brightness;
            _delay = 1000/max_brightness;
            pLedMain->clear(pLedMain, 50);
            DELAY(400);
        }
        color_a = rgb_scale(COLOR_GOLD, brightness);
        color_b = rgb_scale(COLOR_GREEN, brightness);
        pLedMain->set_pixel(pLedMain, 0, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 1, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 2, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 3, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 4, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 5, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 6, color_a.r, color_a.g, color_a.b);
        pLedMain->refresh(pLedMain, 100);
        brightness += delta;
        DELAY(_delay);

        if (brightness >= max_brightness)
        {
            delta = -1;
        }
        else if (brightness < 0)
        {
            delta = 1;
        }

        if (brightness < 0)
        {
            brightness = 0;
        }
    }
    vTaskDelete(NULL);
}

void __bling_loser_task(void *params){
ESP_LOGD(TAG, "Starting loser LED task");
    m_loser_run = true;
    int8_t max_brightness = 255/m_brightness;
    int16_t brightness = 0;
    int8_t delta = 1;
    uint16_t _delay = 2000/max_brightness;
    rgb_t color_a = COLOR_GOLD;
    rgb_t color_b = COLOR_RED;
    while (m_loser_run)
    {
        
        if (brightness==0){
            max_brightness = 255/m_brightness;
            _delay = 1000/max_brightness;
            pLedMain->clear(pLedMain, 50);
            DELAY(400);
        }
        color_a = rgb_scale(COLOR_GOLD, brightness);
        color_b = rgb_scale(COLOR_RED, brightness);
        pLedMain->set_pixel(pLedMain, 0, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 1, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 2, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 3, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 4, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 5, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 6, color_a.r, color_a.g, color_a.b);
        pLedMain->refresh(pLedMain, 100);
        brightness += delta;
        DELAY(_delay);

        if (brightness >= max_brightness)
        {
            delta = -1;
        }
        else if (brightness < 0)
        {
            delta = 1;
        }

        if (brightness < 0)
        {
            brightness = 0;
        }
    }
    vTaskDelete(NULL);
}


void __bling_glimmer_task(void *params){
ESP_LOGD(TAG, "Starting Main LED task");
    m_glimmer_run = true;
    int8_t max_brightness = 255/m_brightness;
    int16_t brightness = 0;
    int8_t delta = 1;
    uint32_t li = 0;
    uint16_t _delay = 1000/max_brightness;
    rgb_t color = COLOR_GOLD;
    while (m_glimmer_run)
    {
        
        if (brightness==0){
            max_brightness = 255/m_brightness;
            _delay = 1000/max_brightness;
            pLedMain->clear(pLedMain, 50);
            DELAY(400);
            li = esp_random() % (LED_COUNT + 1);
        }
        color = rgb_scale(COLOR_GOLD, brightness);
        pLedMain->set_pixel(pLedMain, li, color.r, color.g, 0);
        pLedMain->refresh(pLedMain, 100);
        brightness += delta;
        DELAY(_delay);

        if (brightness >= max_brightness)
        {
            delta = -1;
        }
        else if (brightness < 0)
        {
            delta = 1;
        }

        if (brightness < 0)
        {
            brightness = 0;
        }
    }
    vTaskDelete(NULL);
}

void __bling_special_raffle(void *params){
ESP_LOGD(TAG, "Starting raffle task");
    m_raffle_run = true;
    int8_t max_brightness = 255/m_brightness;
    int16_t brightness_a = 0;
    int8_t delta_a = 1;
    int16_t brightness_b = max_brightness;
    int8_t delta_b = -1;
    uint16_t _delay = 1000/max_brightness;
    rgb_t color_a = COLOR_GOLD;
    rgb_t color_b = COLOR_GREEN;
    bool color_swap=false;
    while (m_raffle_run)
    {
        
        if (brightness_a==0){
            //pLedMain->clear(pLedMain, 50);
            DELAY(400);
            color_swap =!color_swap;
        }
        if (color_swap) {
            color_a = rgb_scale(COLOR_GOLD, brightness_a);
            color_b = rgb_scale(COLOR_GREEN, brightness_b);
        }
        else {
            color_a = rgb_scale(COLOR_GREEN, brightness_a);
            color_b = rgb_scale(COLOR_GOLD, brightness_b);
        }

        pLedMain->set_pixel(pLedMain, 0, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 1, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 2, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 3, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 4, color_a.r, color_a.g, color_a.b);
        pLedMain->set_pixel(pLedMain, 5, color_b.r, color_b.g, color_b.b);
        pLedMain->set_pixel(pLedMain, 6, color_a.r, color_a.g, color_a.b);
        pLedMain->refresh(pLedMain, 100);
        brightness_a += delta_a;
        brightness_b += delta_b;
        DELAY(_delay);

        if (brightness_a >= max_brightness)
        {
            delta_a = -1;
        }
        else if (brightness_a < 0)
        {
            delta_a = 1;
        }

        if (brightness_a < 0)
        {
            brightness_a = 0;
        }
        if (brightness_b >= max_brightness)
        {
            delta_b = -1;
        }
        else if (brightness_b < 0)
        {
            delta_b = 1;
        }

        if (brightness_b < 0)
        {
            brightness_b = 0;
        }
    }
    vTaskDelete(NULL);
}

void __bling_chase_task(void *params)
{
    ESP_LOGD(TAG, "Starting Chase LED task");
    m_led_main_chase_run = true;
    int8_t index = 0;
    rgb_t sel_color = { .r=255, .g= 128, .b = 128};
    rgb_t scaled_color = sel_color;
    while (m_led_main_chase_run)
    {

        scaled_color = rgb_scale(sel_color, 255/m_brightness);
        //TODO : color mode
        pLedMain->set_pixel(pLedMain, index, scaled_color.r, scaled_color.g, scaled_color.b);
        pLedMain->refresh(pLedMain, 100);
        DELAY(m_led_main_chase_delay);
        pLedMain->set_pixel(pLedMain, index, 0, 0, 0);
        if (led_main_chase_forward)
        {
            index++;
            if (index >= LED_COUNT)
                index = 0;
        }
        else
        {
            index--;
            if (index < 0)
                index = LED_COUNT - 1;
        }
    }

    pLedMain->clear(pLedMain, 50);
    vTaskDelete(NULL);
}


void __bling_official_task(void *params)
{
    ESP_LOGD(TAG, "Starting Chase LED task");
    m_official_run = true;
    int8_t index = 0;
    rgb_t sel_color = COLOR_RED;
    bool is_red = true;
    rgb_t scaled_color = sel_color;
    while (m_official_run)
    {
        if (index == 0){
            if (is_red){
                sel_color = COLOR_BLUE;
                is_red = false;
            } else {
                sel_color = COLOR_RED;
                is_red = true;
            }
        }
        scaled_color = rgb_scale(sel_color, 255/m_brightness);
        //TODO : color mode
        pLedMain->set_pixel(pLedMain, index, scaled_color.r, scaled_color.g, scaled_color.b);
        pLedMain->refresh(pLedMain, 100);
        DELAY(m_led_main_chase_delay);
        pLedMain->set_pixel(pLedMain, index, 0, 0, 0);
        if (led_main_chase_forward)
        {
            index++;
            if (index >= LED_COUNT)
                index = 0;
        }
        else
        {
            index--;
            if (index < 0)
                index = LED_COUNT - 1;
        }
    }

    pLedMain->clear(pLedMain, 50);
    vTaskDelete(NULL);
}
void __bling_entrant_task(void *params)
{
    ESP_LOGD(TAG, "Starting Chase LED task");
    m_entrant_run = true;
    int8_t index = 0;
    rgb_t sel_color = COLOR_GOLD;
    bool is_gold = true;
    rgb_t scaled_color = sel_color;
    while (m_entrant_run)
    {
        if (index == 0){
            if (is_gold){
                sel_color = COLOR_GREEN;
                is_gold = false;
            } else {
                sel_color = COLOR_GOLD;
                is_gold = true;
            }
        }
        scaled_color = rgb_scale(sel_color, 255/m_brightness);
        //TODO : color mode
        pLedMain->set_pixel(pLedMain, index, scaled_color.r, scaled_color.g, scaled_color.b);
        pLedMain->refresh(pLedMain, 100);
        DELAY(m_led_main_chase_delay);
        pLedMain->set_pixel(pLedMain, index, 0, 0, 0);
        if (led_main_chase_forward)
        {
            index++;
            if (index >= LED_COUNT)
                index = 0;
        }
        else
        {
            index--;
            if (index < 0)
                index = LED_COUNT - 1;
        }
    }

    pLedMain->clear(pLedMain, 50);
    vTaskDelete(NULL);
}

void __bling_glitch_task(void *params)
{
    ESP_LOGD(TAG, "Starting Glitch!");
    m_glitch_run = true;
    uint32_t max_increments = BLING_GLITCH_MAX_DELAY / BLING_GLITCH_MIN_DELAY;

    while (m_glitch_run)
    {
        uint32_t increments = 0;
        uint32_t leds_to_change = esp_random() % (LED_COUNT + 2); // give it better chance to do full wipe
        uint32_t delay_offset = esp_random() % BLING_GLITCH_MIN_DELAY;
        uint32_t bias = esp_random() % 256;
        if (bias < 240)
        {
            increments = esp_random() % 3;
        }
        else if (bias < 250)
        {
            increments = (esp_random() % (max_increments / 3)) + (max_increments / 3);
        }
        else
        {
            increments = (esp_random() % (max_increments / 2)) + (max_increments / 2);
        }
        // ESP_LOGD(TAG, "Bias: %d --- Increments: %d", bias, increments);
        //  pixel loop
        for (int i = 0; i <= leds_to_change; i++)
        {
            uint32_t o = (esp_random() % 100);
            uint32_t r = (esp_random() % 64) * 4;
            uint32_t g = (esp_random() % 64) * 4;
            uint32_t b = (esp_random() % 64) * 4;
            uint32_t li = esp_random() % (LED_COUNT + 1);
            if (o > 75)
            {
                r = 0;
                g = 0;
                b = 0;
            }
            // if (r <=8 || g <=8 || b <=8) {
            //     r=0;
            //     g=0;
            //     b=0;
            // }
            // ESP_LOGD(TAG, "Setting R%02xG%02xB%02x at %d",r,g,b,li);
            pLedMain->set_pixel(pLedMain, li, r / m_brightness, g / m_brightness, b / m_brightness);
        }
        pLedMain->refresh(pLedMain, 100);
        // delay loop
        for (int i = 0; i <= increments && m_glitch_run; i++)
        {
            DELAY(BLING_GLITCH_MIN_DELAY);
        }
        // final delay
        DELAY(delay_offset);
    }

    vTaskDelete(NULL);
}

int *__p_bling_task(bling_mode_t mode)
{
    switch (mode)
    {
    case bling_rotate_count: // shouldnt get called, but just in case
    case bling_chase:
        return &__bling_chase_task;
        break;
    case bling_starfield:
        return &__bling_starfield_task;
        break;
    case bling_spiral:
    case bling_g_loop:
    case bling_pulse_slow:
    case bling_pulse_fast:
    case bling_heartbeat: //  UNLOCK
    case bling_breathe:
    
    case bling_official: // UNLOCK
        return &__bling_official_task;
        break;
    case bling_glimmer:  // DEFAULT
        return &__bling_glimmer_task;
        break;
    case bling_entrant:  // Entrant Default __CHECK__
        return &__bling_entrant_task;
        break;
    case bling_winner:
        return &__bling_winner_task;
        break;
    case bling_scan:
    case bling_solid:
    case bling_none:
        return -1;
    case bling_special_raffle:
        return &__bling_special_raffle;
        break;
    case bling_special_pulse_green:
    case bling_special_pulse_red:
        return &__bling_loser_task;
        break;
    case bling_glitch: // UNLOCK
        return &__bling_glitch_task;
        break;
    
    default:
        return -1;
    }
}

void bling_disable()
{
    bling_enabled = false;
}

void bling_enable()
{
    bling_enabled = true;
}

void bling_override(bling_mode_t bling_mode){
    
    int *p_bling_fn;
        p_bling_fn = __p_bling_task(bling_mode);
        if (p_bling_fn != -1)
        {
            ESP_LOGD(TAG, "starting bling task");
            xTaskCreatePinnedToCore(p_bling_fn, "Bling", 4096, NULL,
                                    TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);
        }
}

void bling_start()
{
    if (bling_enabled)
    {
        int *p_bling_fn;
        p_bling_fn = __p_bling_task(p_state->bling_mode);
        if (p_bling_fn != -1)
        {
            ESP_LOGD(TAG, "starting bling task");
            xTaskCreatePinnedToCore(p_bling_fn, "Bling", 4096, NULL,
                                    TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);
        }
    }
}

void bling_stop()
{
    // stop them all
    
    m_led_main_chase_run = false;
    m_led_main_blink_run = false;
    m_glimmer_run = false;
    m_glitch_run = false;
    m_raffle_run = false;
    m_winner_run = false;
    m_loser_run = false;
    m_starfield_run = false;
    m_entrant_run = false;
    m_official_run = false;
    // give things time to stop hopefully?
    DELAY(500);
    pLedMain->clear(pLedMain, 50);
}

void bling_set_brightness(led_brightness_mode_t brightness)
{
    switch (brightness)
    {
    case brightness_0:
        m_brightness = 64;
        break;
    case brightness_1:
        m_brightness = 48;
        break;
    case brightness_2:
        m_brightness = 32;
        break;
    case brightness_3:
        m_brightness = 16;
        break;
    case brightness_4:
        m_brightness = 8;
        break;
    case brightness_5:
        m_brightness = 4;
        break;
    case brightness_6:
        m_brightness = 2;
        break;
    case brightness_7:
        m_brightness = 1;
        break;
    }
}

void bling_rotate()
{
    bling_stop();
    pLedMain->clear(pLedMain, 50);
    bling_mode_t _mode = p_state->bling_mode;

    _mode++;
    switch (_mode)
    {
    case bling_rotate_count:
        p_state->bling_mode = 0;
    case bling_chase:
        p_state->bling_mode = bling_chase;
        break;
    case bling_starfield:
        p_state->bling_mode = bling_starfield;
        break;
    case bling_spiral:
        p_state->bling_mode = bling_spiral;
    case bling_g_loop:
        p_state->bling_mode = bling_g_loop;
    case bling_pulse_slow:
        p_state->bling_mode = bling_pulse_slow;
    case bling_pulse_fast:
        p_state->bling_mode = bling_pulse_fast;
    case bling_heartbeat:
        if ((state_is_unlocked(UNLOCK_BEACON_A) &&
          state_is_unlocked(UNLOCK_BEACON_B) && 
          state_is_unlocked(UNLOCK_BEACON_C) &&
          state_is_unlocked(UNLOCK_BEACON_D) &&
          state_is_unlocked(UNLOCK_BEACON_E)) || BADGE_TYPE == BADGE_TYPE_MASTER)
        {
            p_state->bling_mode = bling_heartbeat;
            break;
        }
    case bling_breathe:
        p_state->bling_mode = bling_breathe;
    case bling_glitch: // UNLOCK
        if(state_is_unlocked(UNLOCK_MASTER_INNOC) || state_is_unlocked(UNLOCK_TROLL_INNOC) || BADGE_TYPE == BADGE_TYPE_MASTER  || BADGE_TYPE == BADGE_TYPE_TROLL) {
            p_state->bling_mode = bling_glitch;
            break;
        }
    case bling_official: // UNLOCK
        if(BADGE_TYPE_TROLL || BADGE_TYPE_MASTER) {
            p_state->bling_mode = bling_official;
            break;
        }
    case bling_glimmer: // DEFAULT
        p_state->bling_mode = bling_glimmer;
        break;
    case bling_entrant:
        if(BADGE_TYPE_ENTRANT || BADGE_TYPE_MASTER) {
            p_state->bling_mode = bling_entrant;
            break;
        }
    case bling_winner:
        if(state_is_unlocked(UNLOCK_WINNER) || BADGE_TYPE_MASTER ) {
            p_state->bling_mode = bling_winner;
            break;
        }
    case bling_scan:
        p_state->bling_mode = bling_scan;
    case bling_solid:
        p_state->bling_mode = bling_solid;
    case bling_none:
        p_state->bling_mode = bling_none;
        break;
    default:
        p_state->bling_mode = 0;
        break;
    }
    ESP_LOGD(TAG, "Rotated to %d", p_state->bling_mode);

    bling_start();

}

void led_ui_brightness(int index)
{
    ESP_LOGI(TAG, "Setting brightness pattern %d!", index);
    bling_set_brightness(index);
    for (int i = 0; i < LED_COUNT; ++i)
    {
        pLedMain->set_pixel(pLedMain, i, 255 / m_brightness, 255 / m_brightness, 255 / m_brightness); // /1
    }
#ifndef CONFIG_BOARD_TYPE_DEV
    // red indicator for adjustment level
    if (index > 0)
    {
        pLedMain->set_pixel(pLedMain, index - 1, 255 / m_brightness, 0, 0); // /1
    }

#endif
    pLedMain->refresh(pLedMain, 100);
}

void led_init(void)
{
    ESP_LOGI(TAG, "Initializing LEDs");
    m_mutex = xSemaphoreCreateMutexStatic(&m_mutex_buffer);
    pLedMain = led_strip_init(1, GPIO_LED_MAIN, 8);
    pLedEye = led_strip_init(0, GPIO_LED_EYE, 1);
    /* Set all LED off to clear all pixels */
    pLedMain->clear(pLedMain, 50);
    pLedEye->clear(pLedEye, 50);
}

void bling_init()
{
    ESP_LOGI(TAG, "Initializing Bling");
    p_state = state_bling_get();
    bling_set_brightness(p_state->brightness_mode);
    ESP_LOGD(TAG, "Brightness initially set to :%d", p_state->brightness_mode);
}

void bling_state_reset()
{
    ESP_LOGD(TAG, "%s", __func__);
    bling_state_t *state = state_bling_get();
    state->brightness_mode = BLING_BRIGHTNESS_DEFAULT;
    state->bling_mode = bling_glimmer;
    if (util_check_entrant())
        state->bling_mode = bling_entrant;
}

void bling_cancel_interrupt(){
    if (bling_priority_interrupted) {
        bling_priority_interrupted= false;
        ui_enable();
        bling_stop();
        ui_start();
        bling_start();
    }
}

void bling_interrupt_priority(bling_mode_t bling_mode)
{
    if(!bling_priority_interrupted) {
        bling_priority_interrupted = true;
        ui_start();
        bling_stop();
        DELAY(500);
        ui_disable();
        bling_override(bling_mode);
    }
}

void bling_interrupt(bling_mode_t bling_mode, int delay, raffle_capture_target_t target)
{
    if (!bling_interrupted)
    {
        if (target != RAFFLE_CAPTURE_NONE) set_capture_target(target);
        bling_interrupted = true;
        bling_stop();

        bling_override(bling_mode);
        // start
        DELAY(delay);
        set_capture_target(RAFFLE_CAPTURE_NONE);
        if (!bling_priority_interrupted){
            bling_stop();
            bling_start();
            bling_interrupted = false;
        }
    }
    else
    {
        ESP_LOGD(TAG, "Cannot interrupt. Bling was already interrupted.");
    }
}

void __pulse_led_eye_task(void *params)
{
    // Pulse Count, Pulse Length ms, Max Intensity, Color
    ESP_LOGD(TAG, "Starting eye LED task");
    m_led_eye_pulse_run = true;
    int16_t brightness = 0;
    int8_t delta = 1;

    while (m_led_eye_pulse_run)
    {
        // pthread_mutex_lock(&mtx_led_eye);
        switch (led_eye_pulse_color)
        {
        case EYE_RED:
            
            break;
        case EYE_GREEN:
            pLedEye->set_pixel(pLedEye, 0, 0, brightness, 0);
            break;
        case EYE_BLUE:
            pLedEye->set_pixel(pLedEye, 0, 0, 0, brightness);
            break;

        case EYE_WHITE:
            pLedEye->set_pixel(pLedEye, 0, brightness, brightness, brightness);
            break;
        default:
            break;
        }

        pLedEye->refresh(pLedEye, 100);
        brightness += delta;
        DELAY(led_eye_pulse_delay);

        if (brightness >= led_eye_pulse_max_intensity)
        {
            delta = -1;
        }
        else if (brightness < 0)
        {
            delta = 1;
        }

        if (brightness < 0)
        {
            brightness = 0;
        }
        // pthread_mutex_unlock(&mtx_led_eye);
    }
    pLedEye->clear(pLedEye, 50);
    vTaskDelete(NULL);
}

void led_main_blink_delay(uint16_t delay)
{
    m_led_main_blink_delay = delay;
}

void led_main_chase_color(led_color_t color)
{
    m_led_main_chase_color = color;
}
void led_main_blink_start()
{
    if (!m_led_main_blink_run)
    {
        led_main_all_stop();
        xTaskCreatePinnedToCore(&__bling_blink_task, "LED Blink", 4096, NULL,
                                TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);
    }
}

void led_main_blink_stop()
{
    m_led_main_blink_run = false;
}

void led_main_chase_start()
{
    if (!m_led_main_chase_run)
    {
        led_main_all_stop();
        xTaskCreatePinnedToCore(&__bling_chase_task, "LED Chase", 4096, NULL,
                                TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);
    }
}

void led_main_chase_stop()
{
    m_led_main_chase_run = false;
}

void led_main_chase_swap()
{
    led_main_chase_forward = !led_main_chase_forward;
}

void led_eye_pulse_cycle()
{
    switch (led_eye_pulse_color)
    {
    case EYE_RED:
        led_eye_pulse_color = EYE_GREEN;
        break;
    case EYE_GREEN:
        led_eye_pulse_color = EYE_BLUE;
        break;
    case EYE_BLUE:
        led_eye_pulse_color = EYE_WHITE;
        break;

    case EYE_WHITE:
        led_eye_pulse_color = EYE_RED;
        break;
    default:
        break;
    }
}

void led_main_all_stop()
{
    // all the patterns
    ESP_LOGD(TAG, "Stopping all main LED Eye Patterns");
    m_led_main_blink_run = false;
    m_led_main_chase_run = false;
    m_glimmer_run = false;
}

void led_pulse_configure(uint16_t pulse_length, uint16_t pulse_intensity, led_color_t pulse_color)
{
    int16_t delay = pulse_length / 2 / pulse_intensity;
    led_eye_pulse_color = pulse_color;
    led_eye_pulse_max_intensity = pulse_intensity;
    led_eye_pulse_delay = delay;
}

void led_eye_pulse_start()
{
#ifndef CONFIG_BOARD_TYPE_DEV
    if (!m_led_eye_pulse_run)
    {
        xTaskCreatePinnedToCore(&__pulse_led_eye_task, "Eye LED Pulse", 4096, NULL,
                                TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);
    }
#endif
}

void led_eye_pulse_stop()
{
    m_led_eye_pulse_run = false;
}
