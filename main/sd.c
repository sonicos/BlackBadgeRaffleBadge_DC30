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

static const char *TAG = "RAFFLE::SD";
static volatile bool m_mounted = false;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
const char *base_path = "/spiflash";

static void __mount();
static void __unmount();

void sd_print_space()
{
  FATFS *fs;
  DWORD fre_clust, fre_sect, tot_sect;
  FRESULT res;
  /* Get volume information and free clusters of drive 0 */
  res = f_getfree("0:", &fre_clust, &fs);
  /* Get total sectors and free sectors */
  tot_sect = (fs->n_fatent - 2) * fs->csize;
  fre_sect = fre_clust * fs->csize;

  /* Print the free space (assuming 512 bytes/sector) */
  ESP_LOGD(TAG, "%10lu KiB total drive space.\n%10lu KiB available.\n",
           tot_sect / 2, fre_sect / 2);
}

/**
 * @brief Check for availability of OTA files and reboot into boot_stub if
 * necessary
 */
static void __ota_check()
{
  //   if (util_file_exists(OTA_FIRMWARE_BIN_FILE) &&
  //       util_file_exists(OTA_FIRMWARE_SHA256_FILE)) {
  //     ESP_LOGD(TAG, "OTA files located, attempting to OTA.");
  //     //TODO: OTA Stuff
  //     //ota_reboot_and_flash();
  //   } else {
  //     ESP_LOGD(TAG, "No OTA files found, continuing");
  //   }
}

esp_err_t esp_vfs_fat_info(const char *base_path,
                           uint64_t *out_total_bytes,
                           uint64_t *out_free_bytes)
{

  FATFS *fs;
  DWORD free_clusters;
  int res = f_getfree(base_path, &free_clusters, &fs);
  if (res != FR_OK)
  {
    ESP_LOGE(TAG, "Failed to get number of free clusters (%d)", res);
    // errno = fresult_to_errno(res);
    return ESP_FAIL;
  }
  uint64_t total_sectors = ((uint64_t)(fs->n_fatent - 2)) * fs->csize;
  uint64_t free_sectors = ((uint64_t)free_clusters) * fs->csize;
  WORD sector_size = FF_MIN_SS; // 512
#if FF_MAX_SS != FF_MIN_SS
  sector_size = fs->ssize;
#endif

  // Assuming the total size is < 4GiB, should be true for SPI Flash
  if (out_total_bytes != NULL)
  {
    *out_total_bytes = total_sectors * sector_size;
  }
  if (out_free_bytes != NULL)
  {
    *out_free_bytes = free_sectors * sector_size;
  }
  return ESP_OK;
}
/**
 * Mount the SD card
 */
static void __mount()
{
  if (m_mounted)
  {
    return;
  }

  ESP_LOGD(TAG, "Mounting");

  // Mount config
  esp_vfs_fat_mount_config_t mount_config = {
      .format_if_mount_failed = true, .max_files = 10, .allocation_unit_size = CONFIG_WL_SECTOR_SIZE};

  // Actually attempt to mount the card
  esp_err_t ret = esp_vfs_fat_spiflash_mount(base_path, "vfs", &mount_config, &s_wl_handle);

  if (ret != ESP_OK)
  {
    if (ret == ESP_FAIL)
    {
      ESP_LOGE(TAG,
               "Failed to mount filesystem. "
               "If you want the card to be formatted, set "
               "format_if_mount_failed = true.");
    }
    else
    {
      ESP_LOGE(TAG,
               "Failed to initialize the card (%d). "
               "I mean... this SHOULDNT happen",
               ret);
    }

    // post_state_get()->sd_card_peripheral = false;
    return;
  }

  // post_state_get()->sd_card_peripheral = true;

  ESP_LOGI(TAG, "SD Card Mounted");
  m_mounted = true;

  // Check for OTA files and reboot if necessary
  __ota_check();

  // Print FAT FS size information
  uint64_t bytes_total, bytes_free;
  esp_vfs_fat_info(base_path, &bytes_total, &bytes_free);
  ESP_LOGI(TAG, "FAT FS: %lld kB total, %lld kB free", bytes_total / 1024, bytes_free / 1024);

  sd_print_space();
}

/**
 * Unmount the SD card
 */
static void __unmount()
{
  if (!m_mounted)
  {
    return;
  }

  ESP_ERROR_CHECK(esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
  m_mounted = false;
  ESP_LOGI(TAG, "SD Card Unmounted");
}

void drv_sd_init()
{

  __mount();
}

bool drv_sd_mounted()
{
  return m_mounted;
}
void drv_sd_unmount(){
  __unmount();
}