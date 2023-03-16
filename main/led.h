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

#ifndef COMPONENTS_LED_H_
#define COMPONENTS_LED_H_


#ifdef CONFIG_BOARD_TYPE_DEV
#define LED_COUNT 5
#define GPIO_LED_EYE -1
#define GPIO_LED_MAIN 8
#endif

#ifdef CONFIG_BOARD_TYPE_ESPDEV
#define LED_COUNT 7
#define GPIO_LED_EYE 8
#define GPIO_LED_MAIN 5
#endif

#ifdef CONFIG_BOARD_TYPE_PROD
#define LED_COUNT 7
#define GPIO_LED_EYE 4
#define GPIO_LED_MAIN 5
#endif

#define BLING_BRIGHTNESS_DEFAULT 3

#define BLING_GLITCH_MIN_DELAY 25
#define BLING_GLITCH_MAX_DELAY 1500



// Enumeration used for round robining bling

typedef enum
{
  bling_chase = 0,
  // spin
  // waxon
  // jump
  // coil
  // agoatse
  // starfield
  // smile
  bling_starfield,
  bling_spiral,
  bling_g_loop,
  bling_pulse_slow,
  bling_pulse_fast,
  bling_heartbeat, // 0.4 pulse to pulse, 90 BPM -> UNLOCK
  bling_breathe,
  bling_glitch,   // Fill random, step random, order random -> UNLOCK
  bling_official, // police, etc -> UNLOCK
  bling_glimmer,  // DEFAULT
  bling_entrant,  // Entrant Default
  bling_winner, // unlocks when winner selected
  bling_scan,
  bling_solid,
  bling_none,
  bling_rotate_count, // Counter of production rotation modes
  bling_special_raffle,
  bling_special_pulse_green,
  bling_special_pulse_red,
  
} bling_mode_t;

typedef enum
{
  eye_pulse,
  eye_solid,
  eye_strobe,
  eye_flicker,
  eye_fire,
  eye_glitch,

} eye_mode_t;

typedef enum
{
  EYE_RED,
  EYE_GREEN,
  EYE_BLUE,
  EYE_WHITE,
  EYE_RAINBOW
} eye_color_t;

typedef enum
{
  brightness_0 = 0,
  brightness_1,
  brightness_2,
  brightness_3,
  brightness_4,
  brightness_5,
  brightness_6,
  brightness_7,
} led_brightness_mode_t;

typedef struct
{
  bling_mode_t bling_mode;
  led_brightness_mode_t brightness_mode;
  uint8_t bling_chase_pattern;
  uint8_t bling_starfield_pattern;
  uint8_t bling_spiral_pattern;
  uint8_t bling_g_loop_pattern;
  uint8_t bling_pulse_slow_pattern;
  uint8_t bling_pulse_fast_pattern;
  uint8_t bling_heartbeat_pattern;
  uint8_t bling_breathe_pattern;
  uint8_t bling_glitch_pattern;
  uint8_t bling_official_pattern;
  uint8_t bling_glimmer_pattern;
  uint8_t bling_special_entrant_pattern;
  uint8_t bling_scan_pattern;
  uint8_t bling_solid_pattern;
  // Used to keep a sane state in case I add more
  uint8_t reserved[32];

} bling_state_t;

typedef enum
{
  LED_RED,
  LED_PH1,
  LED_PH2,
  LED_PH3,
  LED_PH4,
} led_color_t;
extern void led_main_blink_delay(uint16_t delay);
extern void led_main_blink_start();
extern void led_main_blink_stop();
extern void led_main_chase_start();
extern void led_main_chase_stop();
extern void led_main_chase_swap();
extern void led_main_all_stop();
extern void led_main_chase_color(led_color_t color);
void led_eye_pulse_cycle();

void led_ui_brightness(int index);
extern void bling_state_reset();
extern void bling_init();
void bling_stop();
extern void bling_rotate();

extern void led_main_set_pixel();

/**
 * Initialize the eye LED
 */
extern void led_init();
extern void bling_set_brightness(led_brightness_mode_t brightness);
extern void led_pulse_configure(uint16_t pulse_length, uint16_t pulse_intensity, led_color_t pulse_color);
extern void led_eye_pulse_start();
void bling_cancel_interrupt();
void bling_interrupt_priority(bling_mode_t bling_mode);
void bling_interrupt(bling_mode_t bling_mode, int delay, raffle_capture_target_t target);
/**
 * Stop pulsing the eye LED
 */
extern void led_eye_pulse_stop();
extern void bling_start();
#endif