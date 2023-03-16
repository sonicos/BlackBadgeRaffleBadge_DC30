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

#define ADV_CONFIG_FLAG (1 << 0)
//#define BROADCAST_DATA_TYPE 0xDC

#define NONCE_CYCLE_TIME (4 * 60 * 1000) /* Every 4 minutes */

#define BLE_ADVERTISEMENT_EVENT_QUEUE_SIZE 10

const static char *TAG = "RAFFLE::BLE";
static uint8_t m_adv_config_done = 0;
static xQueueHandle m_advertisement_evt_queue = NULL;
static bool process_advertisements = false;


static special_ble_mode_t m_special_ble_mode = special_ble_mode_none;

// AES data
#define KEY_SIZE 128

static unsigned char m_aes_time_key[KEY_SIZE / 8] = {
    0, 0, 0, 0, 0x50, 0xeb, 0xc2, 0x87,
    0xb6, 0x8b, 0xcd, 0xfb, 0x79, 0x49, 0x9a, 0x75};
static unsigned char m_aes_raffle_key[KEY_SIZE / 8] = {
    0, 0, 0, 0, 0x3f, 0xcb, 0xa7, 0x10,
    0xf2, 0xcc, 0x00, 0x42, 0x69, 0xbe, 0xef, 0xad};
static unsigned char m_aes_mesh_key[KEY_SIZE / 8] = {
    0, 0, 0, 0, 0x2a, 0x13, 0x7f, 0x5a,
    0x2c, 0x29, 0x60, 0x1d, 0x42, 0xea, 0x3d, 0x0f};

static esp_aes_context m_aes_encrypt_ctx;
static esp_aes_context m_aes_decrypt_ctx;
// static esp_aes_context m_aes_pi_decrypt_ctx;

// Local storage of broadcast data to rotate through
static ble_rotation_mode_t m_rotation_mode;
static ble_advertisement_general_t m_adv_general;
static ble_advertisement_time_t m_adv_time;
static ble_advertisement_mesh_t m_adv_mesh;

static esp_ble_adv_params_t m_adv_params = {
    .adv_int_min = 0x80,
    .adv_int_max = 0xA0,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_data_t m_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x100,
    .max_interval = 0x320,
    .appearance = APPEARANCE_BADGELIFE,
    .manufacturer_len = sizeof(m_adv_general),
    .p_manufacturer_data = (uint8_t *)&m_adv_general,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 32,
    .p_service_uuid = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_PASSIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0xA0, // 100ms
    .scan_window = 0x20,    // 20ms
    .scan_duplicate = BLE_SCAN_DUPLICATE_ENABLE
};

/**
 * @brief Update the current advertising data with the global data
 */
static void __adv_data_set()
{
  // config adv data

  esp_err_t ret = esp_ble_gap_config_adv_data(&m_adv_data);
  

  if (ret)
  {
    ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
  }
  // ESP_LOGD(TAG,"adv done - %d", ret);
  m_adv_config_done |= ADV_CONFIG_FLAG;
}

/**
 * @brief Parse an alt beacon advertisement. Manufacturer data has already been
 * parsed by the dc25() function and garanteed to be 26 bytes
 * @param result : Pointer to result structure containing the data
 */
static inline void __parse_alt_beacon(uint8_t *data_bytes)
{
  
}

/**
 * Parse the name out of a BLE GAP advertisement into a peer
 *
 * @return true if a valid DC25 peer
 */
static inline bool __parse_badgelife_old(
    peer_t *p_peer,
    struct ble_scan_result_evt_param *result,
    uint16_t appearance)
{
  bool valid = false;
  uint8_t data_length;

  // Parse manufacturer data
  uint8_t *data_bytes = esp_ble_resolve_adv_data(
      result->ble_adv, ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, &data_length);
  if (data_bytes != NULL)
  {
    if (data_length == DATA_LENGTH_DC25 || 13)
    {
      uint16_t company_id = 0;
      memcpy(&company_id, data_bytes, 2);
      p_peer->company_id = company_id;

      ESP_LOGD(TAG, "%s company id %04x = 0x%02x", LOG_YELLOW, appearance, company_id);

      // Only valid if company id is parsable and it's known
      valid =
          (company_id == PEERS_COMPANY_ID_ANDNXOR ||
           company_id == PEERS_COMPANY_ID_BLINKYBLING ||
           company_id == PEERS_COMPANY_ID_CPV ||
           company_id == PEERS_COMPANY_ID_DC503 ||
           company_id == PEERS_COMPANY_ID_DCDARKNET ||
           company_id == PEERS_COMPANY_ID_DC801 ||
           company_id == PEERS_COMPANY_ID_DCFURS ||
           company_id == PEERS_COMPANY_ID_DCZIA ||
           company_id == PEERS_COMPANY_ID_GROUND_CONTROL ||
           company_id == PEERS_COMPANY_ID_JOCO ||
           company_id == PEERS_COMPANY_ID_TRANS_IONOSPHERIC1 ||
           company_id == PEERS_COMPANY_ID_TRANS_IONOSPHERIC2);
    }
    else if (data_length == DATA_LENGTH_ALT_BEACON)
    {
      //__parse_alt_beacon(data_bytes);
    }
  }

  // Parse name
  if (valid)
  {
    uint8_t *name_bytes = esp_ble_resolve_adv_data(
        result->ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &data_length);
    if (name_bytes != NULL)
    {
      memcpy(p_peer->name, name_bytes, MIN(STATE_NAME_LENGTH, data_length));
      p_peer->name[STATE_NAME_LENGTH] = 0; // Ensure null terminated
    }
  }

  return valid;
}

/**
 * Parse the name out of a BLE GAP advertisement into a peer
 *
 * @return true if a valid DC30 peer
 */
static inline bool __parse_badgelife(
    peer_t *p_peer,
    struct ble_scan_result_evt_param *result,
    uint16_t appearance)
{
  uint8_t data_length;
  uint8_t name_length;
  uint8_t *name_bytes = esp_ble_resolve_adv_data(
      result->ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &name_length);
  if (name_bytes == NULL)
  {
    ESP_LOGD(TAG, "NULL NAME - DC30");
    return false;
  }
  uint8_t *data_bytes = esp_ble_resolve_adv_data(
      result->ble_adv, ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, &data_length);
  if (data_length != sizeof(ble_advertisement_general_t))
  {
    ESP_LOGD(TAG,
             "Invalid badgelife DC26 packet length. Length: %d Expected: %d",
             data_length, sizeof(ble_advertisement_general_t));
    // return false;
    //  try to parse it as an older defcon one
    return __parse_badgelife_old(p_peer, result, appearance);
  }

  // Get the company id
  uint16_t company_id = 0;
  memcpy(&company_id, data_bytes, 2);
  p_peer->company_id = company_id;

  // if its not mine, dont treat it as mine
  if (company_id != PEERS_COMPANY_ID_BLACKBADGE)
  {
    return __parse_badgelife_old(p_peer, result, appearance);
  }

  ble_advertisement_general_t packet;
  memcpy(&packet, data_bytes, sizeof(ble_advertisement_general_t));
  memcpy(p_peer->name, name_bytes, MIN(STATE_NAME_LENGTH, name_length));
  p_peer->name[STATE_NAME_LENGTH] = 0; // Ensure null terminated
  memcpy(&p_peer->nonce, packet.nonce, 4);

  return true;
}

// /**
//  * @brief Parse a raspberry pi advertisement
//  * @param p_result : Pointer to BLE result
//  */
// static void __parse_pi(struct ble_scan_result_evt_param* p_result) {
//   uint8_t data_length;
//   uint8_t* data_bytes = esp_ble_resolve_adv_data(
//       p_result->ble_adv, ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, &data_length);
//   if (data_length != sizeof(ble_advertisement_pi_t)) {
//     ESP_LOGD(TAG, "Invalid PI packet length. Length: %d Expected: %d",
//              data_length, sizeof(ble_advertisement_pi_t));
//     return;
//   }

//   ESP_LOGD(TAG, "Raspberry PI Data");
//   ESP_LOG_BUFFER_HEX(TAG, data_bytes, data_length);

//   // Populate the pi time struct
//   ble_advertisement_pi_t pi;
//   memcpy(&pi, data_bytes, data_length);

//   ESP_LOGD(TAG, "Struct PI Data");
//   ESP_LOG_BUFFER_HEX(TAG, &pi, data_length);

//   // Build a key
//   unsigned char key[KEY_SIZE / 8];
//   memcpy(key, m_aes_pi_key, KEY_SIZE / 8);
//   memcpy(key, pi.nonce, 4);
//   ESP_LOGD(TAG, "Private key");
//   ESP_LOG_BUFFER_HEX(TAG, key, KEY_SIZE / 8);
//   esp_aes_setkey(&m_aes_pi_decrypt_ctx, key, KEY_SIZE);

//   // Decrypt
//   esp_aes_crypt_ecb(&m_aes_pi_decrypt_ctx, ESP_AES_DECRYPT, data_bytes + 2,
//                     (unsigned char*)&pi.seconds);

//   ESP_LOGD(TAG, "Struct PI Decrypted Data");
//   ESP_LOG_BUFFER_HEX(TAG, &pi.seconds, 12);

//   uint32_t crc = crc32_le(0, (unsigned char*)&pi.seconds, 12);
//   ESP_LOGD(TAG, "Computed CRC: 0x%08x", crc);
//   uint32_t pi_crc;
//   memcpy(&pi_crc, pi.crc, sizeof(uint32_t));

//   if (crc == __bswap_32(pi_crc)) {
//     ESP_LOGD(TAG, "%sValid PI Packet Received", LOG_GREEN);
//   } else {
//     ESP_LOGE(TAG, "Invalid PI Packet");
//     return;
//   }

//   // Byte swap seconds
//   *(uint32_t*)pi.seconds = __bswap_32(*(uint32_t*)pi.seconds);
//   *(uint32_t*)pi.useconds = __bswap_32(*(uint32_t*)pi.useconds) / 1000;

//   // Byte swap version
//   uint16_t version = pi.version[0] << 8 | pi.version[1];
//   ESP_LOGD(TAG, "Pi Version: %d My version %d", version, VERSION_INT);

//   ESP_LOGD(TAG, "Decrypted PI Data");
//   ESP_LOGD(TAG, "Company ID: 0x%04x", pi.company_id);
//   ESP_LOGD(TAG, "Seconds: %u uSec: %u", *(uint32_t*)pi.seconds,
//            *(uint32_t*)pi.useconds);
//   ESP_LOGD(TAG, "Seconds:0x%08x uSec: 0x%08x", *(uint32_t*)pi.seconds,
//            *(uint32_t*)pi.useconds);
//   ESP_LOGD(TAG, "Stratum: %d Padding: 0x%02x 0x%02x 0x%02x", pi.stratum,
//            pi.padding[0], pi.padding[0], pi.padding[0]);
//   ESP_LOGD(TAG, "CRC: 0x%08x Nonce: 0x%08x", *(uint32_t*)pi.crc,
//            *(uint32_t*)pi.nonce);

//   // Attemp to sync with the time source
//   time_manager_sync(*(uint32_t*)pi.seconds, *(uint32_t*)pi.useconds,
//                     pi.stratum);

//   // Try to download an OTA on every beacon, this is not a good idea. But... OTA
//   // will handle the logic of limiting connections to the AP and download
//   //if (version > VERSION_INT) {
//   //  xTaskCreatePinnedToCore(ota_download_task, "OTA", 6000, NULL,
//   //                          TASK_PRIORITY_HIGH, NULL, APP_CPU_NUM);
//   //}
// }

static void __handle_scan_response(struct ble_scan_result_evt_param *result,
                                   uint16_t appearance)
{
  /* 
  This is here so that we wait until after everything is initialized and loaded before
  we start processing. If badges were nearby at boot time, the advertisement could have
  been processed before we loaded peers or state, causing a serious headache. This will
  prevent that issue.
  */
  if (!process_advertisements) return; 
  // ESP_LOGD(TAG,"About to allocate");
  uint8_t *cipher_text = util_heap_alloc_ext(16);
  uint8_t *clear_text = util_heap_alloc_ext(16);
  memset(cipher_text, 0, 16);
  memset(clear_text, 0, 16);

  uint8_t *manufacturer_data;
  uint8_t length = 0;
  bool store_badge = false;
  peer_t peer;
  memset(&peer, 0, sizeof(peer));

  // Convert address (6 byte array) to a long
  peer.address = *(uint64_t *)result->bda & 0xFFFFFFFFFFFF;
  // ESP_LOGD(TAG,"Get Time manager");
  peer.timestamp = time_manager_now_sec();
  // ESP_LOGD(TAG,"Get Peer");
  peer_t *p_known_peer = peers_get_by_address(peer.address);
  //	esp_log_buffer_hex(TAG, result->bda, 6);
  //

  // ESP_LOGD(TAG, "%sAppearance 0x%02x", LOG_YELLOW, appearance);

  switch (appearance)
  {
  case APPEARANCE_RAFFLE:
    // RAFFLE!
    /**
     * Handle raffle advertisements
     */
    manufacturer_data = esp_ble_resolve_adv_data(
        result->ble_adv, ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, &length);
    if (length != sizeof(ble_advertisement_raffle_t))
    {
      ESP_LOGD(TAG, "%s Invalid raffle packet length %d expected %d",
               __func__, length, sizeof(ble_advertisement_raffle_t));
      break;
    }

    ble_advertisement_raffle_t raffle_packet;

    // Decrypt raffle packet if it's a known peer
    if (p_known_peer != NULL)
    {
      memcpy(cipher_text, manufacturer_data,
             sizeof(ble_advertisement_raffle_t));

      unsigned char key[KEY_SIZE / 8];
      memcpy(key, m_aes_raffle_key, KEY_SIZE / 8);
      memcpy(key, &p_known_peer->nonce, 4);
      esp_aes_setkey(&m_aes_decrypt_ctx, key, KEY_SIZE);

#ifdef CONFIG_DEBUG_BLE
      ESP_LOGD(TAG, "%sReceived Raffle Cipher Text", LOG_CYAN);
      ESP_LOG_BUFFER_HEXDUMP(TAG, cipher_text, 16, ESP_LOG_DEBUG);
      ESP_LOGD(TAG, "%sDECRYPT KEY", LOG_CYAN);
      ESP_LOG_BUFFER_HEX(TAG, m_aes_decrypt_ctx.key, 32);
#endif

      esp_aes_crypt_ecb(&m_aes_decrypt_ctx, ESP_AES_DECRYPT, cipher_text,
                        clear_text);

      // ESP_LOGD(TAG, "%sBotnet DECRYPTED Clear Text", LOG_CYAN);
      // ESP_LOG_BUFFER_HEX(TAG, clear_text, 16);

      memcpy(&raffle_packet, clear_text, sizeof(ble_advertisement_raffle_t));
      raffle_advertiser_handler(&raffle_packet);
    }
    break;
  case APPEARANCE_NONE:
    break;
  case APPEARANCE_BADGELIFE_DC25:
  case APPEARANCE_BADGELIFE_DC26:
  case APPEARANCE_BADGELIFE_DC27:
  // case APPEARANCE_BADGELIFE_DC28:
  case APPEARANCE_BADGELIFE_DC29:
  case APPEARANCE_BADGELIFE:
    store_badge = __parse_badgelife(&peer, result, appearance);
    break;
  case APPEARANCE_MESH:
    // Mesh!
    /**
     * Handle mesh advertisements
     */
    manufacturer_data = esp_ble_resolve_adv_data(
        result->ble_adv, ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, &length);
    if (length != sizeof(ble_advertisement_mesh_t))
    {
      ESP_LOGD(TAG, "%s Invalid mesh packet length %d expected %d",
               __func__, length, sizeof(ble_advertisement_mesh_t));
      break;
    }

    ble_advertisement_mesh_t mesh_packet;

    // Decrypt raffle packet if it's a known peer
    if (p_known_peer != NULL)
    {
      memcpy(cipher_text, manufacturer_data,
             sizeof(ble_advertisement_mesh_t));

      unsigned char key[KEY_SIZE / 8];
      memcpy(key, m_aes_mesh_key, KEY_SIZE / 8);
      memcpy(key, &p_known_peer->nonce, 4);
      esp_aes_setkey(&m_aes_decrypt_ctx, key, KEY_SIZE);

#ifdef CONFIG_DEBUG_BLE
      ESP_LOGD(TAG, "%sReceived Mesh Cipher Text", LOG_CYAN);
      ESP_LOG_BUFFER_HEXDUMP(TAG, cipher_text, 16, ESP_LOG_DEBUG);
      ESP_LOGD(TAG, "%sDECRYPT KEY", LOG_CYAN);
      ESP_LOG_BUFFER_HEX(TAG, m_aes_decrypt_ctx.key, 32);
#endif

      esp_aes_crypt_ecb(&m_aes_decrypt_ctx, ESP_AES_DECRYPT, cipher_text,
                        clear_text);

      // ESP_LOGD(TAG, "%sBotnet DECRYPTED Clear Text", LOG_CYAN);
      // ESP_LOG_BUFFER_HEX(TAG, clear_text, 16);

      memcpy(&mesh_packet, clear_text, sizeof(ble_advertisement_mesh_t));
      mesh_advertiser_handler(&mesh_packet);
    }
    break;

    // Handle time sync from another badge
  case APPEARANCE_TIME:;
  #ifndef CONFIG_BADGE_TYPE_BEACON // we dont need no stinkin external updates
    manufacturer_data = esp_ble_resolve_adv_data(
        result->ble_adv, ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, &length);
    if (length != sizeof(ble_advertisement_time_t))
    {
      ESP_LOGD(TAG, "%s Invalid BLE Time length %d expected %d", __func__,
               length, sizeof(ble_advertisement_time_t));
      break;
    }

    memcpy(cipher_text, manufacturer_data, sizeof(ble_advertisement_time_t));

    // Decrypt broadcast packet if it's a known peer

    if (p_known_peer != NULL)
    {
      unsigned char key[KEY_SIZE / 8];
      memcpy(key, m_aes_time_key, KEY_SIZE / 8);
      memcpy(key, &p_known_peer->nonce, 4);
      esp_aes_setkey(&m_aes_decrypt_ctx, key, KEY_SIZE);
      esp_aes_crypt_ecb(&m_aes_decrypt_ctx, ESP_AES_DECRYPT, cipher_text,
                        clear_text);

#ifdef CONFIG_DEBUG_BLE
      ESP_LOGD(TAG, "%sTime Received Cipher Text", LOG_CYAN);
      ESP_LOG_BUFFER_HEXDUMP(TAG, cipher_text,
                             sizeof(ble_advertisement_time_t), ESP_LOG_DEBUG);
      ESP_LOGD(TAG, "%sTime Received Clear Text", LOG_CYAN);
      ESP_LOG_BUFFER_HEXDUMP(TAG, &clear_text,
                             sizeof(ble_advertisement_time_t), ESP_LOG_DEBUG);
#endif

      ble_advertisement_time_t ble_time;
      memcpy(&ble_time, clear_text, sizeof(ble_advertisement_time_t));
      time_manager_advertisement_handle(ble_time);
    }
    #endif
    break;
  default:
    ESP_LOGD(TAG, "Unmatched Peer appearance = 0x%02x", appearance);
  }

  if (store_badge)
  {
    peers_update_or_create(&peer, true);
  }

  free(cipher_text);
  free(clear_text);
}

static void __scan_response_handler_task(void *parameters)
{
  struct ble_scan_result_evt_param result;
  while (1)
  {
    if (xQueueReceive(m_advertisement_evt_queue, &result, portMAX_DELAY))
    {
      // ESP_LOGD(TAG, "got BT adv");
      uint8_t appearance_length;
      uint8_t *p_appearance = esp_ble_resolve_adv_data(
          result.ble_adv, ESP_BLE_AD_TYPE_APPEARANCE, &appearance_length);

      // Ensure appearance is at least a 0
      uint16_t appearance = 0;
      if (p_appearance != NULL)
      {
        appearance = p_appearance[0] | (p_appearance[1] << 8);
      }

      __handle_scan_response(&result, appearance);
    }
    // taskYIELD();
  }

  // Shouldn't get here but just in case
  vTaskDelete(NULL);
}

/**
 * This task changes the nonce in the advertisement periodically
 */
static void __nonce_task(void *parameters)
{
  uint32_t nonce;
  while (1)
  {
    ESP_LOGD(TAG, "%s%s Cycling nonce", LOG_CYAN, __func__);
    nonce = util_random(0, UINT32_MAX);
    memcpy(m_adv_general.nonce, &nonce, 4);

    // Update three keys
    memcpy(m_aes_raffle_key, &nonce, 4);
    memcpy(m_aes_mesh_key, &nonce, 4);
    memcpy(m_aes_time_key, &nonce, 4);

    // Wait some time
    DELAY(NONCE_CYCLE_TIME);
  }

  // Just in case
  vTaskDelete(NULL);
}

/**
 * This is a freeRTOS task that round robins between multiple GAP broadcasts.
 * This effectively lets us advertise a lot more than the 27ish bytes that BLE
 * allows :)
 */
static void IRAM_ATTR __rotate_task(void *parameters)
{
  // Create input / output buffers for encryption
  uint8_t *p_cipher_text = (uint8_t *)util_heap_alloc_ext(16);
  uint8_t *p_clear_text = (uint8_t *)util_heap_alloc_ext(16);

  while (1)
  {
    // Round robin
    m_rotation_mode = (m_rotation_mode + 1) % ble_rotate_count;
    // util_heap_stats_dump();
    // util_nvs_stats_dump();
    switch (m_rotation_mode)
    {
    case ble_rotate_badgelife_mimic:
      // Mimic badgelife badges randomly, if special mode is enabled
      // if (m_special_ble_mode == special_ble_mode_badgelife_mimic)
      // {
      //   uint16_t modes[] = {PEERS_COMPANY_ID_CPV, PEERS_COMPANY_ID_DC801};
      //   uint16_t company_id = modes[util_random(0, 2)];
      //   m_adv_data.p_manufacturer_data = (uint8_t *)&company_id;
      //   m_adv_data.manufacturer_len = sizeof(company_id);
      //   m_adv_data.appearance = APPEARANCE_BADGELIFE;
      //   m_adv_data.include_name = false;
      //   // Set the BLE data
      //   __adv_data_set();

      //   DELAY(4000);
      // }
      break;
    case ble_rotate_general:
      ESP_LOGD(TAG, "Rotate badgelife");
      m_adv_data.p_manufacturer_data = (uint8_t *)&m_adv_general;
      // m_adv_general.magic = state_badgelife_magic_byte_get();
      m_adv_data.manufacturer_len = sizeof(m_adv_general);
      m_adv_data.include_name = true;
      m_adv_data.appearance = APPEARANCE_BADGELIFE;

      // Ensure name is set properly. Other GAP changes it
      char name[STATE_NAME_LENGTH + 1];
      state_name_get(name);
      esp_ble_gap_set_device_name(name);
      // Set the advertising data
      __adv_data_set();
      DELAY(6000);
      break;

    case ble_rotate_raffle:
      ESP_LOGD(TAG, "Rotate raffle");
      memset(p_clear_text, 0, 16);
      memcpy(p_clear_text, raffle_packet_get(),
             sizeof(ble_advertisement_raffle_t));

      // Encrypt the botnet data
      esp_aes_setkey(&m_aes_encrypt_ctx, m_aes_raffle_key, KEY_SIZE);
      esp_aes_crypt_ecb(&m_aes_encrypt_ctx, ESP_AES_ENCRYPT, p_clear_text,
                        p_cipher_text);

      // ESP_LOGD(TAG, "%sBotnet Clear Text", LOG_YELLOW);
      // ESP_LOG_BUFFER_HEXDUMP(TAG, p_clear_text, 16, ESP_LOG_DEBUG);

      // ESP_LOGD(TAG, "%sBotnet Encrypt Key", LOG_YELLOW);
      // ESP_LOG_BUFFER_HEX(TAG, m_aes_encrypt_ctx.key, 32);
      // ESP_LOGD(TAG, "%sBotnet Cipher Text", LOG_YELLOW);
      // ESP_LOG_BUFFER_HEXDUMP(TAG, p_cipher_text, 16, ESP_LOG_DEBUG);

      // Tweak GAP so we can fit the botnet packet
      char buffer[8];
      sprintf(buffer, "BBR%04x", util_serial_get());
      esp_ble_gap_set_device_name(buffer);
      m_adv_data.p_manufacturer_data = p_cipher_text;
      m_adv_data.manufacturer_len = sizeof(ble_advertisement_raffle_t);
      m_adv_data.include_name = false;
      m_adv_data.appearance = APPEARANCE_RAFFLE;

      // Set the advertising data
      __adv_data_set();
      DELAY(6000);

      break;
    case ble_rotate_mesh_state:
      ESP_LOGD(TAG, "Rotate mesh");
      char buf[8];
      sprintf(buf, "BBR%04x", util_serial_get());
      esp_ble_gap_set_device_name(buf);
      m_adv_data.p_manufacturer_data = p_cipher_text;
      m_adv_data.manufacturer_len = sizeof(ble_advertisement_mesh_t);
      m_adv_data.include_name = false;
      m_adv_data.appearance = APPEARANCE_MESH;

      mesh_state_t *state = state_mesh_get();

      m_adv_mesh.timestamp_seconds = state->timestamp_seconds;
      m_adv_mesh.winner_serial = state->winner_serial;
      m_adv_mesh.mesh_flags = state->mesh_flags;
      m_adv_mesh.stratum = state->stratum;
      // Compute the CRC of the first 14 bytes
      *(uint16_t *)m_adv_mesh.crc = crc16_le(
            0, (uint8_t *)&m_adv_mesh, sizeof(ble_advertisement_mesh_t) - 2);

      memset(p_clear_text, 0, 16);
      memcpy(p_clear_text, &m_adv_mesh, sizeof(ble_advertisement_mesh_t));
      // Encrypt the data
      esp_aes_setkey(&m_aes_encrypt_ctx, m_aes_mesh_key, KEY_SIZE);
      esp_aes_crypt_ecb(&m_aes_encrypt_ctx, ESP_AES_ENCRYPT, p_clear_text,
                        p_cipher_text);

      // Set the advertising data
      __adv_data_set();
      DELAY(6000);
      break;

    /**
     *Do time advertisement. This allows other badges to sync up with this
     *badge's time source. The time is changed a few times in hopes that
     *recipient badges receive multiple advertisements and have a precise
     *time sync
     */
    case ble_rotate_time:
      ESP_LOGD(TAG, "Rotate time");
      m_adv_data.include_name = false;
      m_adv_data.appearance = APPEARANCE_TIME;
      m_adv_data.p_manufacturer_data = p_cipher_text;
      m_adv_data.manufacturer_len = sizeof(ble_advertisement_time_t);

      struct timeval time;
      // Advertise 40 different values with a delay of 100ms between each,
      // roughly 4 seconds
      for (uint16_t i = 0; i < 4; i++)
      {
        // Change advertisement to current time
        gettimeofday(&time, NULL);
        memcpy(&m_adv_time.seconds, &time.tv_sec, sizeof(time_t));
        memcpy(&m_adv_time.useconds, &time.tv_usec, sizeof(suseconds_t));
        m_adv_time.stratum = time_manager_stratum_get();
        m_adv_time.canary = BLE_TIME_CANARY;

        // Compute the CRC of the first 14 bytes
        *(uint16_t *)m_adv_time.crc = crc16_le(
            0, (uint8_t *)&m_adv_time, sizeof(ble_advertisement_time_t) - 2);

        // Copy into a buffer we can encrypt
        memset(p_clear_text, 0, 16);
        memcpy(p_clear_text, &m_adv_time, sizeof(ble_advertisement_time_t));

        // Encrypt time data
        esp_aes_setkey(&m_aes_encrypt_ctx, m_aes_time_key, KEY_SIZE);
        esp_aes_crypt_ecb(&m_aes_encrypt_ctx, ESP_AES_ENCRYPT, p_clear_text,
                          p_cipher_text);

        // ESP_LOGD(TAG, "%sTime Clear Text", LOG_YELLOW);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, p_clear_text,
        //                        sizeof(ble_advertisement_time_t),
        //                        ESP_LOG_DEBUG);
        // ESP_LOGD(TAG, "%sTime Cipher Text", LOG_YELLOW);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, p_cipher_text,
        //                        sizeof(ble_advertisement_time_t),
        //                        ESP_LOG_DEBUG);
        __adv_data_set();
        // ESP_LOGD(TAG, "Time iterator: %d",i);
        DELAY(500);
      }
      break;
    case ble_rotate_count:
      // Ignore
      break;
    }
  }

  // Just in case
  vTaskDelete(NULL);
  free(p_cipher_text);
  free(p_clear_text);
}

/**
 * @brief Handle GAP interrupts quickly queueing other tasks to process data
 */
static void IRAM_ATTR gap_event_handler(esp_gap_ble_cb_event_t event,
                                        esp_ble_gap_cb_param_t *param)
{
  switch (event)
  {
#ifdef CONFIG_SET_RAW_ADV_DATA
  case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    m_adv_config_done &= (~ADV_CONFIG_FLAG);
    if (m_adv_config_done == 0)
    {
      esp_ble_gap_start_advertising(&m_adv_params);
    }
    break;
  case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
    m_adv_config_done &= (~scan_rsp_config_flag);
    if (m_adv_config_done == 0)
    {
      esp_ble_gap_start_advertising(&m_adv_params);
    }
    break;
#else
  case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    m_adv_config_done &= (~ADV_CONFIG_FLAG);
    if (m_adv_config_done == 0)
    {
      //ESP_LOGD(TAG,"ADV_DATA_SET -> %d", param->adv_data_cmpl.status);
      // Some goofy ass memory leak I cant track down causes this
      if( param->adv_data_cmpl.status != ESP_BT_STATUS_SUCCESS ) reboot_trigger();
      esp_ble_gap_start_advertising(&m_adv_params);
    }
    break;
#endif

  case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
  {
    // Scan forever
    esp_ble_gap_start_scanning(0);
    break;
  }

  // BLE Scan Started
  case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
    break;

    // BLE Scan Result received
  case ESP_GAP_BLE_SCAN_RESULT_EVT:
  {
    esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
    switch (scan_result->scan_rst.search_evt)
    {
    case ESP_GAP_SEARCH_INQ_RES_EVT:;
      if (xQueueSendToBack(m_advertisement_evt_queue,
                           &scan_result->scan_rst, 0) == errQUEUE_FULL)
      {
        // scan queue full
      }
      break;
    case ESP_GAP_SEARCH_INQ_CMPL_EVT:
      break;
    default:
      break;
    }
    break;
  }

  // BLE Scan Stopped
  case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
    break;

  // GAP Advertisement Start Complete
  case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:

    // ESP_LOGD(TAG, "Start adv status: %x", param->adv_start_cmpl.status);
    break;
  case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
    {
      ESP_LOGE(TAG, "Advertising stop failed\n");
    }
    else
    {
      ESP_LOGD(TAG, "Stop adv successfully\n");
    }
    break;
  case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
    // ESP_LOGD(
    //     TAG,
    //     "update connetion params status = %d, min_int = %d, max_int = "
    //     "%d,conn_int = %d,latency = %d, timeout = %d",
    //     param->update_conn_params.status,
    //     param->update_conn_params.min_int,
    //     param->update_conn_params.max_int,
    //     param->update_conn_params.conn_int,
    //     param->update_conn_params.latency,
    //     param->update_conn_params.timeout);
    break;
  default:
    ESP_LOGD(TAG, "Unhandled event -> %d", event);
    break;
  }
}

void ble_special_mode_set(special_ble_mode_t mode)
{
  m_special_ble_mode = mode;
}

void ble_init()
{
  ESP_LOGD(TAG, "Initializing BLE");
  esp_err_t ret;

  // uint16_t serial = (uint16_t)util_serial_get();

  // Setup advertisement event queue (to rate limit in a saturated
  // environment)
  m_advertisement_evt_queue =
      xQueueCreate(BLE_ADVERTISEMENT_EVENT_QUEUE_SIZE,
                   sizeof(struct ble_scan_result_evt_param));
  TaskHandle_t handle;
  xTaskCreatePinnedToCore(__scan_response_handler_task, "BLE Scan", 4096, NULL,
                          TASK_PRIORITY_BLE, &handle, PRO_CPU_NUM);

  // Setup AES contexts
  esp_aes_init(&m_aes_encrypt_ctx);
  esp_aes_init(&m_aes_decrypt_ctx);
  // esp_aes_init(&m_aes_pi_decrypt_ctx);

  // Setup manufacturer data
  m_adv_general.company_id = BLE_MANUFACTURER_ID;
  // memcpy(m_adv_general.serial, &serial, 2);
  m_adv_general.serial = util_serial_get();
  m_adv_general.flags = 0;
  // m_adv_general.magic = 0;
  uint32_t nonce = util_random(0, UINT32_MAX);
  memcpy(m_adv_general.nonce, &nonce, 4);

  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ret = esp_bt_controller_init(&bt_cfg);
  if (ret)
  {
    ESP_LOGE(TAG, "%s initialize controller failed\n", __func__);
    return;
  }

  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, BLE_POWER_LEVEL);

  ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
  if (ret)
  {
    ESP_LOGE(TAG, "%s enable controller failed\n", __func__);
    return;
  }
  ret = esp_bluedroid_init();
  if (ret)
  {
    ESP_LOGE(TAG, "%s init bluetooth failed\n", __func__);
    return;
  }
  ret = esp_bluedroid_enable();
  if (ret)
  {
    ESP_LOGE(TAG, "%s enable bluetooth failed\n", __func__);
    return;
  }
  ret = esp_ble_gap_register_callback(gap_event_handler);
  if (ret)
  {
    ESP_LOGE(TAG, "gap register error, error code = %x", ret);
    return;
  }

  esp_ble_gap_set_device_name("RAFFLE");
  __adv_data_set();

  // Configure scanning
  esp_err_t scan_ret = esp_ble_gap_set_scan_params(&ble_scan_params);
  if (scan_ret)
  {
    ESP_LOGE(TAG, "set scan params error, error code = %x", scan_ret);
  }
  ESP_LOGI(TAG, "BT Initialized");
}

void ble_unlock_update()
{
  ESP_LOGD(TAG, "Updating BLE Unlock data");
  m_adv_general.flags = state_unlock_get();
  __adv_data_set();
}

void ble_name_set(const char *name)
{
  esp_ble_gap_set_device_name(name);
}

void ble_nonce_task_start()
{
  TaskHandle_t task_handle = NULL;
  static StaticTask_t task;
  task_handle = util_task_create(__nonce_task, "BLE Nonce Task", 4096, NULL,
                                 TASK_PRIORITY_LOW, &task);
  configASSERT(task_handle);
}

void ble_rotate_task_start()
{
  ESP_LOGD(TAG, "Starting BLE GAP rotation task");
  xTaskCreate(__rotate_task, "BLE Task", 4096, NULL, 5, NULL);
  // Because of some memory leak I can't figure out in the rotation, I am making a terrible hack
  reboot_terrible_hack_timer_start();
}

void ble_begin_processing(){
  process_advertisements = true;
}
