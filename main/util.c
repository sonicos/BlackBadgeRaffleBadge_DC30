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
#define SHA_CHUNK_SIZE 512 /* Can be up to 8192 but lets save some stack */
#define SHA256_OUTPUT_SIZE_BYTES 32


const static char *TAG = "UTIL";




static uint32_t _serial = 0;

bool util_file_exists(const char *path)
{
  struct stat s;
  return (stat(path, &s) == 0);
}

bool util_check_entrant()
{
  // TODO: check efuse
  return false;
}

void heap_caps_alloc_failed_hook(size_t requested_size, uint32_t caps, const char *function_name)
{
  printf("%s was called but failed to allocate %d bytes with 0x%X capabilities. \n",function_name, requested_size, caps);
}

void util_file_sha256(char *path, char *hex_digest)
{
  uint8_t hash[SHA256_OUTPUT_SIZE_BYTES];
  uint8_t buffer[SHA_CHUNK_SIZE];
  size_t bytes_read = 0;
  int32_t bytes_left = util_file_size(path);
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);

  FILE *file = fopen(path, "r");
  if (file)
  {
    while (bytes_left > 0)
    {
      bytes_read = fread(buffer, 1, SHA_CHUNK_SIZE, file);
      mbedtls_sha256_update(&ctx, buffer, bytes_read);
      bytes_left -= bytes_read;
    }

    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
  }
  fclose(file);

  // Convert to hex digest string
  memset(hex_digest, 0, SHA256_DIGEST_SIZE_BYTES + 1);
  for (uint8_t i = 0; i < SHA256_OUTPUT_SIZE_BYTES; i++)
  {
    sprintf(hex_digest + (i * 2), "%02x", hash[i]);
  }
}

uint32_t util_file_size(const char *path)
{
  struct stat st;
  stat(path, &st);
  return st.st_size;
}

/**
 * @brief Dump heap stats to DEBUG
 */
void util_heap_stats_dump()
{
  ESP_LOGD(TAG,
           "%sHEAP Free: %d "
           "%sLowest: %d "
           "%sDMA: %d "
           "%sLargest Block: %d",
           LOG_CYAN, xPortGetFreeHeapSize(), LOG_YELLOW,
           xPortGetMinimumEverFreeHeapSize(), LOG_RED,
           heap_caps_get_free_size(MALLOC_CAP_DMA), LOG_BLUE, heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
}

void util_nvs_stats_dump()
{
  // Example of nvs_get_stats() to get the number of used entries and free entries:
  nvs_stats_t nvs_stats;
  nvs_get_stats(NULL, &nvs_stats);
  ESP_LOGD(TAG, "Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n",
           nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);
}

/**
 * @brief Allocate on external memory. Simple helper function
 * @param size : Size of heap block to allocate
 */
inline void *util_heap_alloc_ext(size_t size)
{
  // return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  //  I dont have SPI RAM, so...
  return heap_caps_malloc(size, MALLOC_CAP_DEFAULT);
}

/**
 * @brief Re-allocate space on external memory. Simple helper function
 * @param ptr : Pointer to reallocate memory of
 * @param size: New size of heap block
 */
inline void *util_heap_realloc_ext(void *ptr, size_t size)
{
  // return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM);
  //  I dont have SPI RAM, so...
  return heap_caps_realloc(ptr, size, MALLOC_CAP_DEFAULT);
}

color_rgb_t util_hsv_to_rgb(float H, float S, float V)
{
  H = fmodf(H, 1.0);

  float h = H * 6;
  uint8_t i = floor(h);
  float a = V * (1 - S);
  float b = V * (1 - S * (h - i));
  float c = V * (1 - (S * (1 - (h - i))));
  float rf, gf, bf;

  switch (i)
  {
  case 0:
    rf = V * 255;
    gf = c * 255;
    bf = a * 255;
    break;
  case 1:
    rf = b * 255;
    gf = V * 255;
    bf = a * 255;
    break;
  case 2:
    rf = a * 255;
    gf = V * 255;
    bf = c * 255;
    break;
  case 3:
    rf = a * 255;
    gf = b * 255;
    bf = V * 255;
    break;
  case 4:
    rf = c * 255;
    gf = a * 255;
    bf = V * 255;
    break;
  case 5:
  default:
    rf = V * 255;
    gf = a * 255;
    bf = b * 255;
    break;
  }

  uint8_t R = rf;
  uint8_t G = gf;
  uint8_t B = bf;

  color_rgb_t RGB = (R << 16) + (G << 8) + B;
  return RGB;
}

/**
 * @brief Map a value between in min/max to out min/max
 * @param x : value to map
 * @param in_min : input value minimum
 * @param in_max : input value maximum
 * @param out_min : output value minimum
 * @param out_max : output value maximum
 */
int32_t util_map(int32_t x,
                 int32_t in_min,
                 int32_t in_max,
                 int32_t out_min,
                 int32_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int util_mkdir_p(const char *path)
{
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  const size_t len = strlen(path);
  char _path[PATH_MAX];
  char *p;

  errno = 0;

  /* Copy string so its mutable */
  if (len > sizeof(_path) - 1)
  {
    errno = ENAMETOOLONG;
    return -1;
  }
  strcpy(_path, path);

  /* Iterate the string */
  for (p = _path + 1; *p; p++)
  {
    if (*p == '/')
    {
      /* Temporarily truncate */
      *p = '\0';

      if (mkdir(_path, S_IRWXU) != 0)
      {
        if (errno != EEXIST)
          return -1;
      }

      *p = '/';
    }
  }

  if (mkdir(_path, S_IRWXU) != 0)
  {
    if (errno != EEXIST)
      return -1;
  }

  return 0;
}

void util_nvs_init()
{
  esp_err_t ret;
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
  {
    ESP_LOGE(TAG, "No free pages in NVS, erasing...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

/**
 * @brief Print text to console without hogging the CPU
 * @param text : Text to print
 */
void util_print_nice(char *text)
{
  while (strlen(text) > 0)
  {
    uint32_t len = MIN(strlen(text), 32);
    printf("%.32s", text);
    text += len;
    YIELD();
  }
}

/**
 * Return a random number between min and max-1.
 */
uint32_t util_random(uint32_t min, uint32_t max)
{
  if (max == min)
  {
    return min;
  }
  return (esp_random() % (max - min)) + min;
}

/**
 * Converts a 16-bit 565 color to RGB color suitable for LEDs
 */
color_rgb_t util_565_to_rgb(color_565_t color)
{
  return ((0xF800 & color) << 8) | ((0x07E0 & color) << 5) |
         (0x1F & color) << 3;
}

color_565_t util_rgb_to_565_discreet(uint8_t red, uint8_t green, uint8_t blue)
{
  uint16_t b = (blue >> 3) & 0x1f;
  uint16_t g = ((green >> 2) & 0x3f) << 5;
  uint16_t r = ((red >> 3) & 0x1f) << 11;

  uint16_t c = (uint16_t)(r | g | b);
  return c;
}

color_565_t util_rgb_to_565(color_rgb_t rgb)
{
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;
  return util_rgb_to_565_discreet(r, g, b);
}

/**
 * @brief Get 32 bits of the factory-unique MAC address as a semi-reliable
 * serial number
 * @return 32 bit serial
 */
uint32_t util_serial_get()
{
  // I don't want to have to keep reading the damn efuse
  if (_serial != 0) return _serial;
  uint8_t serial[6];
  esp_efuse_mac_get_default(serial);
  _serial = serial[2] << 24 | serial[3] << 16 | serial[4] << 8 | serial[5];
  return _serial;
}

/**
 * @brief Wrapper to create tasks in a more heap friendly way
 * @param function : Function to run for the task
 * @param name : Name of the task to use throughout the badge
 * @param stack_depth : How much stack to allocate
 * @param parameters : User data to pass to task
 * @param priority : Priority to assign to the task
 * @param p_static_task : Static task reference we should return to
 * @return Newly created task handle
 */
TaskHandle_t util_task_create(TaskFunction_t function,
                              const char *const name,
                              uint32_t stack_depth,
                              void *parameters,
                              UBaseType_t priority,
                              StaticTask_t *p_static_task)
{
  util_heap_stats_dump();

  StackType_t *p_stack =
      (StackType_t *)util_heap_alloc_ext(stack_depth * sizeof(StackType_t));
  ESP_LOGD(TAG, "Creating static task '%s' with stack size %d bytes", name,
           stack_depth);
  util_heap_stats_dump();
  return xTaskCreateStaticPinnedToCore(function, name, stack_depth, parameters,
                                       priority, p_stack, p_static_task,
                                       APP_CPU_NUM);
  //	TaskHandle_t handle;
  //	xTaskCreate(function, name, stack_depth, parameters, priority, &handle);
  //	return handle;
}

/**
 * @brief XORs a string with a key
 * @param input : Input buffer to XOR it may contain erroneous \0
 * @param output : Output buffer to write to, may contain erroneous \0
 * @param length : Actual length of buffer
 * @param key : Key to XOR with
 */
void util_string_xor(const char *input,
                     char *output,
                     size_t length,
                     char *key)
{
  size_t key_len = strlen(key);
  for (size_t i = 0; i < length; i++)
  {
    *output++ = *input++ ^ key[i % key_len];
  }
  *output = '\0';
}

/**
 * @brief Dump task list to terminal
 */
void util_task_stat_dump()
{
  char *buffer = util_heap_alloc_ext(60000);
  vTaskGetRunTimeStats(buffer);
  printf("%s\n\n", buffer);
  free(buffer);
}


/**
 * @brief Validate an unlock code. State is updated
 * @param code : Code to validate
 * @return true if successful
 */
bool util_validate_unlock(const char *code)
{
  bool result = false;

  
  if (result)
  {
    ble_unlock_update();
  }

  return result;
}

/*
 * reallocarray() is not available on ESP32 so we just use the OpenBSD
 * implementation
 *
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW ((size_t)1 << (sizeof(size_t) * 4))

void *reallocarray(void *optr, size_t nmemb, size_t size)
{
  if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && nmemb > 0 &&
      SIZE_MAX / nmemb < size)
  {
    errno = ENOMEM;
    return NULL;
  }
  return realloc(optr, size * nmemb);
}
