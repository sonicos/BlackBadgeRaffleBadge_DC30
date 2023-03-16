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

const static char *TAG = "RAFFLE::Boot";

void post_dump()
{
  ESP_LOGI(TAG, "========================= POST ===========================");
  ESP_LOGI(TAG, "\tSerial: 0x%04x", util_serial_get());
  ESP_LOGI(TAG, "==========================================================");
}

// static void on_button(button_t *btn, button_state_t state)
// {
//     ESP_LOGI(TAG, "%s button %s", btn == &btn1 ? "First" : "Second", states[state]);
//     if(state == BUTTON_CLICKED){
//         ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
//         blink_led();
//         /* Toggle the LED state */
//         s_led_state = !s_led_state;
//     }
// }

/**
 * @brief Do a normal boot
 */
__attribute__((__unused__)) static void __normal_boot()
{
  ESP_LOGI(TAG, "Normal Boot!");
  // led_main_chase_start();
  led_pulse_configure(2000, 64, EYE_BLUE);

  // Startup BLE GAP rotation task
  state_save_task_start();
  ble_rotate_task_start();

  // Startup BLE Nonce randomization task
  ble_nonce_task_start();
  // start tasks

#ifdef CONFIG_DEVELOPER_MODE
  // Run bootloader menu as a task so we keep main running in bg
  //__developer_mode();
#else

  // Check if recently updated
  //__update_check();

  // Load badge state
  state_load();

  peers_load();

  #ifdef CONFIG_BADGE_TYPE_MASTER
  drawings_load();
  #endif
  // Check bagel pin unlock
  // util_bagel_pin_unlock_check();

  // if (btn_down()) {
  //  gfx_fill_screen(COLOR_RED);
  //   gfx_push_screen_buffer();
  //   while (btn_down()) {
  //     // wait for button release
  //   }
  //   btn_clear();

  //  __developer_mode();
  //} else {
  // Start the addons
  // addons_task_start();

  // Intro video :)
  // gfx_play_raw_file("/sdcard/bling/intro.raw", 0, 0, LCD_WIDTH, LCD_HEIGHT,NULL, false, NULL);

  // Startup Console
  // console_task_start();

  // Indicate in logs that things are running
  
  post_dump();
  #if CONFIG_BBR_ENABLE_WIFI
  // TODO: Enable Wifi
  //wifi_enable();
  #endif
  ESP_LOGI(TAG, "Black Badge Raffle Badge Ready - %s - Now: %d", VERSION, time_manager_now_sec());

  
  led_eye_pulse_stop();
  bling_init();
  ui_init();
  ui_start();
  #ifndef CONFIG_BADGE_TYPE_BEACON
  // nofun4u
  console_task_start();
  #endif
  DELAY(5000);
  ble_begin_processing();

  while (1)
  {
    // ui_menu_main(NULL);
    DELAY(1000);
    taskYIELD();
    // esp_task_wdt_reset();
  }

  // Just in case
  esp_restart();

  // Should never happen
  while (1)
  {
    DELAY(50);
  }
  //}
#endif
}

/**
 * @brief Do a test boot
 */
__attribute__((__unused__)) static void __test_boot()
{

  ESP_LOGI(TAG, "Hardware Test Boot!");
  led_main_chase_color(EYE_RED);
  led_pulse_configure(1000, 50, EYE_RED);
  led_main_chase_start();
  led_eye_pulse_start();
  DELAY(1000);
  led_main_chase_color(EYE_GREEN);
  led_pulse_configure(1000, 50, EYE_GREEN);
  DELAY(1000);
  led_main_chase_color(EYE_BLUE);
  led_pulse_configure(1000, 50, EYE_BLUE);
  DELAY(1000);
  led_main_chase_color(EYE_WHITE);
  led_pulse_configure(1000, 50, EYE_WHITE);
  DELAY(1000);
  led_main_chase_color(EYE_RED);
  led_pulse_configure(3000, 50, EYE_RED);

  ESP_LOGI(TAG, "Black Badge Raffle Hardware Test Initialized - %s", VERSION);
  // First button connected between GPIO and GND
  // pressed logic level 0, no autorepeat

  ui_init();
  ui_testmode();

  // btn_test_mode();
  //  L button changes color
  //  R button changes direction

  while (1)
  {
    DELAY(10);
    taskYIELD();
  }

  // should not happen
  esp_restart();

  // Should better not happen
  while (1)
  {
    DELAY(50);
    taskYIELD();
  }
}

/**
 * @brief Startup the badge
 */
void raffle_bootloader()
{
  heap_caps_register_failed_alloc_callback(heap_caps_alloc_failed_hook);
  // initialialize core devices
  led_init();
  led_eye_pulse_start();
  buttons_init();
  util_nvs_init();
  drv_sd_init();
  DELAY(500); // wait for FS mount

#ifndef CONFIG_BADGE_TYPE_HWTEST
  // SEEKRET BOOT MODE
  if (button_boot_check())
  {
    ESP_LOGI(TAG, "SEEKRET HW TEST MODE!");
    __test_boot();
  }
  // Do normal startup
  // start tasks/services
  // led_main_blink_delay(100);

  // led_main_blink_start();
  util_heap_stats_dump();
  mesh_init();
  state_init();
  ble_init();
  // gap_sync_init();

  peers_init();
  raffle_init();
  led_pulse_configure(3000, 60, EYE_GREEN);
  #ifdef CONFIG_BADGE_TYPE_MASTER
  drawings_init();
  #endif
  __normal_boot();

#else
  __test_boot();
#endif

  // Everything should fall here on main task
  while (1)
  {
    DELAY(6000);
    ESP_LOGI(TAG, "Proc!");
  }
}
