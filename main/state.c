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

const static char *TAG = "RAFFLE::state";
static badge_state_t m_state;
static SemaphoreHandle_t m_mutex;
static volatile bool m_state_changed;


// Just some encryption stuff
#define KEY_SIZE 128
#define IV_SIZE 16
esp_aes_context m_state_aes;
static unsigned char m_state_key[KEY_SIZE / 8] = {
    0xa6, 0x0c, 0xb9, 0xfa, 0x31, 0x32, 0x51, 0x05,
    0xa8, 0xc2, 0x8e, 0x43, 0x10, 0xc5, 0x67, 0x6b};

static unsigned char m_state_iv[IV_SIZE] = {0xb8, 0x18, 0xbe, 0x4b, 0x4b, 0x2a,
                                            0x27, 0x26, 0x96, 0x5e, 0xc4, 0x73,
                                            0xf0, 0x67, 0x85, 0xfd};

static void __write_to_sd()
{
  // Setup output and IV buffers
  uint8_t output[sizeof(badge_state_t)];
  unsigned char temp_iv[IV_SIZE];
  memcpy(temp_iv, m_state_iv, IV_SIZE);

  // Make sure SD card is inserted
  if (!drv_sd_mounted())
  {
    ESP_LOGE(TAG, "SD card not mounted, cannot save state.");
    return;
  }

  // Store the time for future
  m_state.time_sec = time_manager_now_sec();
  ESP_LOGD(TAG, "Saving time %u - 0x%04x default: 0x%04x", m_state.time_sec,
           m_state.time_sec, TIME_DEFAULT);

  // Compute the CRC
  m_state.crc = crc32_le(0, &m_state, sizeof(badge_state_t) - 4);

  // ESP_LOGD(TAG, "Plain Data");
  // ESP_LOG_BUFFER_HEX(TAG, &m_state, sizeof(badge_state_t));
  // ESP_LOGD(TAG, "State Size: %d", sizeof(badge_state_t));

  // ESP_LOGD(TAG, "IV Before:");
  // ESP_LOG_BUFFER_HEX(TAG, m_state_iv, IV_SIZE);

  // Encrypt the data, updating IV
  esp_aes_crypt_cbc(&m_state_aes, ESP_AES_ENCRYPT, sizeof(badge_state_t),
                    temp_iv, &m_state, output);

  // ESP_LOGD(TAG, "IV After:");
  // ESP_LOG_BUFFER_HEX(TAG, temp_iv, IV_SIZE);
  //ESP_LOGD(TAG, "Encrypted Data");
  //ESP_LOG_BUFFER_HEX(TAG, output, sizeof(badge_state_t));
  //ESP_LOGD(TAG, "Sizr of output: %d", sizeof(output));

  // Open the state file
  FILE *file = fopen(STATE_FILE, "w");
  if (!file)
  {
    ESP_LOGE(TAG, "Cannot open badge state file");
    return;
  }

  // Write encrypted output and IV
  size_t wb;
  int success;
  sd_print_space();
  wb = fwrite(output, sizeof(badge_state_t), 1, file);
  //ESP_LOGD(TAG, "wrote: %d", sizeof(wb));
  fwrite(m_state_iv, IV_SIZE, 1, file);
  //ESP_LOGD(TAG, "wrote: %d", sizeof(wb));
  success = fclose(file);
  //ESP_LOGD(TAG, "Success?: %d", success);
  //ESP_LOGD(TAG, "Size of saved file: %d", util_file_size(STATE_FILE));

  // Update the IV now that CBC has done its thing
  memcpy(m_state_iv, temp_iv, IV_SIZE);
}

/**
 * @brief Task that runs in the background attempting to save state when
 * necessary
 */
void __save_task(void *parameters)
{
  uint32_t last_save = time_manager_now_sec();
  while (1)
  {
    DELAY(30000);

    // Wait for a flag to tell us state has changed, or it's been at least 5 minutes
    if (time_manager_now_sec() > last_save + 300 || m_state_changed)
    {
      reboot_prevent(reboot_save);
      ESP_LOGD(TAG, "%s Saving", __func__);
      peers_save();
      #ifdef CONFIG_BADGE_TYPE_MASTER
      drawings_save();
      #endif
      __write_to_sd();
      // __write_to_nvs();
      last_save = time_manager_now_sec();
      // Invalidate save indicator
      m_state_changed = false;
      reboot_enable(reboot_save);
    }
  }
  // Just in case?
  vTaskDelete(NULL);
}

/**
 * Initilize badge state handler. This does not load any data, merely gets
 * things ready
 */
void state_init()
{
  memset(&m_state, 0, sizeof(m_state));
  state_name_set(STATE_NAME_DEFAULT);
  m_mutex = xSemaphoreCreateMutex();

  esp_aes_init(&m_state_aes);
  esp_aes_setkey(&m_state_aes, m_state_key, KEY_SIZE);
  ESP_LOGD(TAG, "Badge state initialized");
}

/**
 * @brief Reset badge state to new
 */
void state_new()
{
  memset(&m_state, 0, sizeof(m_state));
  ESP_LOGD(TAG, "Badge state reset");
  m_state.unlock_state = STATE_UNLOCK_DEFAULT;
  m_state.badgelife_magic_byte = STATE_BADGELIFE_MAGIC_BYTE_DEFAULT;
  #if defined(CONFIG_BADGE_TYPE_TROLL) || defined(CONFIG_BADGE_TYPE_MASTER)
  m_state.adv_flags = RAFFLE_ADV_GLITCH;
  #endif
  m_state.bling_state.bling_mode = bling_glimmer;
  m_state.time_sec = TIME_DEFAULT;
  time_manager_set_time(TIME_DEFAULT, 0, TIME_MAX_STRATUM);
  // set default name to serial
  char *name = (char *)malloc((STATE_NAME_LENGTH + 1) * sizeof(char));
  #ifdef CONFIG_BADGE_TYPE_BEACON
  sprintf(name, "BBRBCN%02x", util_serial_get() & 0xFF);
  #else
  sprintf(name, "BBR%04x", util_serial_get() & 0xFFFF);
  #endif
  state_name_set(name);
  free(name);

  botnet_reset();

  bling_state_reset();
  mesh_state_reset();
  // initialize_console_data();
  //  DO NOT SAVE HERE. This is usually called from state_load which shares a
  //  semaphore with state_save
  // ui_pick_name();
}

// botnet_state_t* state_botnet_get() {
//   return &(m_state.botnet_state);
// }

// game_data_t* state_console_get() {
//   return &(m_state.console_state);
// }

/**
 * Load badge state from NVS
 */
void state_load()
{
  ESP_LOGD(TAG, "%s", __func__);

  // Make sure SD card is inserted
  if (!drv_sd_mounted())
  {
    ESP_LOGE(TAG, "SD card not mounted, cannot load state.");
    state_new();
    return;
  }

  badge_state_t temp_state;
  uint8_t input[sizeof(badge_state_t)];

  // Make sure file size is exactly how we expect it
  if (util_file_size(STATE_FILE) != (sizeof(badge_state_t) + IV_SIZE))
  {
    ESP_LOGE(TAG, "Invalid badge state file - Got: %d --- Expecting: %d", util_file_size(STATE_FILE), (sizeof(badge_state_t) + IV_SIZE));
    state_new();
    return;
  }

  FILE *file = fopen(STATE_FILE, "r");
  if (!file)
  {
    ESP_LOGE(TAG, "Cannot open badge state file");
    state_new();
    return;
  }

  // Read IV and encrypted data into temp buffer
  fread(input, sizeof(badge_state_t), 1, file);
  fread(m_state_iv, IV_SIZE, 1, file);
  // ESP_LOGD(TAG, "Encrypted Data:");
  // ESP_LOG_BUFFER_HEX(TAG, input, sizeof(badge_state_t));
  // ESP_LOGD(TAG, "Read IV:");
  // ESP_LOG_BUFFER_HEX(TAG, m_state_iv, IV_SIZE);
  fclose(file);

  // Decrypt the state into a tempory badge state
  esp_aes_crypt_cbc(&m_state_aes, ESP_AES_DECRYPT, sizeof(badge_state_t),
                    m_state_iv, input, &temp_state);
  // ESP_LOGD(TAG, "Decrypted Data:");
  // ESP_LOG_BUFFER_HEX(TAG, &temp_state, sizeof(badge_state_t));
  // ESP_LOGD(TAG, "IV After Decrypt:");
  // ESP_LOG_BUFFER_HEX(TAG, m_state_iv, IV_SIZE);

  // Check CRC
  uint32_t computed_crc = crc32_le(0, &temp_state, sizeof(badge_state_t) - 4);
  if (temp_state.crc != computed_crc)
  {
    ESP_LOGE(TAG, "Corrupted badge state file - CRC");
    state_new();
    return;
  }

  // Ensure name length is valid
  if (strlen(temp_state.name) == 0)
  {
    ESP_LOGE(TAG, "Corrupted badge state file - NAME");
    state_new();
    return;
  }

  // We got this far, state checks out, allow it to be used
  memcpy(&m_state, &temp_state, sizeof(badge_state_t));

  ESP_LOGD(TAG, "Badge state loaded");

  // Ensure minimum time
  if (time_manager_now_sec() < TIME_DEFAULT)
  {
    time_manager_set_time(TIME_DEFAULT, 0, TIME_MAX_STRATUM);
  }

  // Load time from badge
  ESP_LOGD(TAG, "%sTemp time=%u Stored time=%u now=%u", LOG_RED,
           temp_state.time_sec, m_state.time_sec, time_manager_now_sec());
  if (m_state.time_sec > time_manager_now_sec())
  {
    ESP_LOGD(TAG, "State has a better time, using that");
    time_manager_set_time(m_state.time_sec, 0, TIME_MAX_STRATUM);
  }
  ble_unlock_update();
  return ESP_OK;
}

/**
 * @brief Get the current brightness mode
 */
// led_brightness_mode_t state_brightness_get() {
//   return m_state.brightness_mode;
// }

// /**
//  * @brief Set the badgelife magic byte
//  */
// void state_brightness_set(led_brightness_mode_t mode) {
//   m_state.brightness_mode = mode;
// }

/**
 * @brief Get the current badgelife magic byte
 */
uint8_t state_badgelife_magic_byte_get()
{
  return m_state.badgelife_magic_byte;
}

/**
 * @brief Set the badgelife magic byte
 */
void state_badgelife_magic_byte_set(uint8_t magic_byte)
{
  m_state.badgelife_magic_byte = magic_byte;
}

// uint8_t state_brightness_get() {
//   return m_state.brightness;
// }

// void state_brightness_set(uint8_t brightness) {
//   brightness = MAX(brightness, STATE_BRIGHTNESS_MIN);
//   brightness = MIN(brightness, STATE_BRIGHTNESS_MAX);
//   m_state.brightness = brightness;
// }

// bool state_hide_bling_get() {
//   return m_state.hide_bling;
// }

// void state_hide_bling_set(bool hide) {
//   m_state.hide_bling = hide;
// }

void state_name_get(char *name)
{
  snprintf(name, STATE_NAME_LENGTH + 1, "%s", m_state.name);
}

void state_name_set(char *name)
{
  snprintf(m_state.name, STATE_NAME_LENGTH + 1, "%s", name);
  ble_name_set(m_state.name);
}

/**
 * @brief Indicate to badge that something has changed in state and it needs
 * saving
 */
void state_save_indicate()
{
  m_state_changed = true;
}

/**
 * @brief Run a task in the background to handle saving state (also saves peers)
 */
void state_save_task_start()
{
  static StaticTask_t task;
  util_task_create(__save_task, "Save", 4096, NULL, TASK_PRIORITY_MEDIUM,
                   &task);
}

/**
 * @brief Get the current high pong score
 */
// uint32_t state_score_pong_get() {
//   return m_state.score_pong;
// }

// /**
//  * @brief Set the current high pong score
//  */
// void state_score_pong_set(uint32_t score) {
//   m_state.score_pong = score;
// }
bling_state_t *state_bling_get()
{
  return &(m_state.bling_state);
}
mesh_state_t *state_mesh_get()
{
  return &(m_state.mesh_state);
}

uint8_t *state_adv_flags_get()
{
  return &(m_state.adv_flags);
}
/**
 * @brief Get the current high ski score
 */
// uint32_t state_score_ski_get() {
//   return m_state.score_ski;
// }

// /**
//  * Set the curren high ski free score
//  */
// void state_score_ski_set(uint32_t score) {
//   m_state.score_ski = score;
// }

/**
 * @brief Get the current tilt enabled state
 */
// bool state_tilt_get() {
//   return m_state.tilt_enabled;
// }

void state_raffle_adv_flag_add(raffle_adv_flags_t flag)
{
}

void state_raffle_adv_flag_remove(raffle_adv_flags_t flag)
{
}


void state_unlock(uint16_t unlock)
{
  m_state.unlock_state |= unlock;
  ble_unlock_update();
  raffle_unlock_update();
  state_save_indicate();
}


// /**
//  * @brief Enable or disable the screen tilt
//  */
// void state_tilt_set(bool tilt) {
//   m_state.tilt_enabled = tilt;
// }
/**
 * @brief Get the current unlock state
 * @return Current unlock state as a bitmask
 */
uint16_t state_unlock_get()
{
  return m_state.unlock_state;
}
/**
 * @brief Set the current unlock state
 * @param unlock : The unlock state as a bitmask
 */
void state_unlock_set(uint16_t unlock)
{
  m_state.unlock_state = unlock;
  ble_unlock_update();
  raffle_unlock_update();
  state_save_indicate();
}

bool state_is_unlocked(uint16_t flag)
{
  return (m_state.unlock_state & flag);;
}