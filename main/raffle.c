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
const static char *TAG = "RAFFLE::Raffle";


volatile static ble_advertisement_raffle_t m_raffle_packet = {
    .unlock_flags = 0,
    .badge_type = BADGE_TYPE,
    .adv_flags = 0,
    .random = 42069, //nice
    .serial = {0x77, 0x77},
};

static bool m_payload_stop = false;
static uint32_t m_timestamp = 0;
static xQueueHandle m_payload_queue = NULL;
static uint8_t m_adv_update = 0;
static uint8_t m_adv_flags = 0;
static uint8_t *p_flags;
static raffle_capture_target_t m_capture_target = RAFFLE_CAPTURE_NONE;

void set_capture_target(raffle_capture_target_t target){
  m_capture_target = target;
}

void raffle_capture_target(){
  if(m_capture_target == RAFFLE_CAPTURE_MASTER && !state_is_unlocked(UNLOCK_MASTER_INNOC)){
    state_unlock(UNLOCK_MASTER_INNOC);
    state_unlock(UNLOCK_TROLL_INNOC);
    
    m_capture_target = RAFFLE_CAPTURE_NONE;
    led_pulse_configure(2000,40,EYE_RED);
    led_eye_pulse_start();
    bling_stop();
    DELAY(1000);
    bling_start();
    DELAY(4000);
    led_eye_pulse_stop();
  }
  else if (m_capture_target == RAFFLE_CAPTURE_TROLL && !state_is_unlocked(UNLOCK_TROLL_INNOC)){
    state_unlock(UNLOCK_TROLL_INNOC);
    led_eye_pulse_stop();
    m_capture_target = RAFFLE_CAPTURE_NONE;
    led_pulse_configure(2000,40,EYE_GREEN);
    bling_stop();
    DELAY(1000);
    bling_start();
    DELAY(4000);
    led_eye_pulse_stop();
  }

}


static IRAM_ATTR void __adv_task(void *parameters)
{
  uint8_t adv_copy = 0;
  
  while (1)
  {
    if (m_adv_update == 0){
      DELAY(500);
      continue;
    }
    
    
    adv_copy = m_adv_update; // copy it over
    m_adv_update = 0; // clear the original so that it can take in new update
    m_adv_flags = *p_flags;
    m_adv_flags |= adv_copy;
    // Eye indicator
    led_pulse_configure(3000, 50, EYE_GREEN);
    led_eye_pulse_start();
    DELAY(CONFIG_RAFFLE_ADV_FLAG_TIMEOUT);
    led_eye_pulse_stop();
    m_adv_flags = *p_flags;
    adv_copy=0;

  }
  // Shouldn't get here but just in case
  vTaskDelete(NULL);
}



static void __payload_unlocks_delete()
{
  ESP_LOGD(TAG, "%sDeleting unlocks :( :( :(", LOG_RED);
  state_unlock_set(0);
}

static IRAM_ATTR void __payload_task(void *parameters)
{
  raffle_adv_flags_t flags;
  while (1)
  {
    // Discard queue so we get immediate response on next bling
    xQueueReceive(m_payload_queue, &flags, 1 / portTICK_PERIOD_MS);

    // Wait for a successful attack to come through, then execute it
    if (xQueueReceive(m_payload_queue, &flags, portMAX_DELAY))
    {
      ESP_LOGD(TAG,"Got queue");
      if (flags & RAFFLE_ADV_CAPTURE_M){
        ESP_LOGD(TAG,"Capture Master");
        m_capture_target = RAFFLE_CAPTURE_MASTER;
        led_pulse_configure(3000, 60, EYE_RED);
        led_eye_pulse_start();
      } else if (flags & RAFFLE_ADV_CAPTURE_T){
        ESP_LOGD(TAG,"Capture Troll");
        m_capture_target = RAFFLE_CAPTURE_TROLL;
        led_pulse_configure(3000, 60, EYE_GREEN);
        led_eye_pulse_start();
      }
    }

    // m_current_payload = botnet_payload_none;
    //  Limit how often payloads can run
    DELAY(30000);
    led_eye_pulse_stop();
    m_capture_target = RAFFLE_CAPTURE_NONE;
  }
  // Shouldn't get here but just in case
  vTaskDelete(NULL);
}

void raffle_advertiser_handler(ble_advertisement_raffle_t *packet)
{
  // TODO: Handle Raffle Advertisement
  if(raffle_payload_validate(packet)){
    
    #ifdef CONFIG_BADGE_TYPE_MASTER
    ESP_LOGD(TAG,"Storing drawing data");
    drawing_t drawing = {
      .serial = packet->serial,
      .badge_type = packet->badge_type,
      .unlock_flags = packet->unlock_flags
    };
    drawings_process(drawing);
    #endif
    char flags[16];
    itoa(packet->adv_flags, flags, 2);
    ESP_LOGD(TAG,"Got Adv -> Serial: %8x Type: %u Flags: %s",packet->serial, packet->badge_type, flags);
    if (packet->serial == BEACON_A_SERIAL && !state_is_unlocked(UNLOCK_BEACON_A)) state_unlock(UNLOCK_BEACON_A);
    if (packet->serial == BEACON_B_SERIAL && !state_is_unlocked(UNLOCK_BEACON_B)) state_unlock(UNLOCK_BEACON_B);
    if (packet->serial == BEACON_C_SERIAL && !state_is_unlocked(UNLOCK_BEACON_C)) state_unlock(UNLOCK_BEACON_C);
    if (packet->serial == BEACON_D_SERIAL && !state_is_unlocked(UNLOCK_BEACON_D)) state_unlock(UNLOCK_BEACON_D);
    if (packet->serial == BEACON_E_SERIAL && !state_is_unlocked(UNLOCK_BEACON_E)) state_unlock(UNLOCK_BEACON_E);
    if(packet->adv_flags & RAFFLE_ADV_GLITCH && !(BADGE_TYPE == BADGE_TYPE_MASTER || BADGE_TYPE == BADGE_TYPE_TROLL || BADGE_TYPE == BADGE_TYPE_TROLL)){
      if (packet->badge_type == BADGE_TYPE_TROLL && !state_is_unlocked(UNLOCK_TROLL_INNOC)){
        bling_interrupt(bling_glitch,30000, RAFFLE_CAPTURE_TROLL);
      } else if (packet->badge_type == BADGE_TYPE_MASTER && !state_is_unlocked(UNLOCK_MASTER_INNOC)){
        bling_interrupt(bling_glitch,45000, RAFFLE_CAPTURE_MASTER);
      }
    }
    //xQueueSend(m_payload_queue, &packet->adv_flags, 1 / portTICK_PERIOD_MS);
  }
}


void raffle_unlock_update()
{
  // TODO: Update raffle payload flags
}

ble_advertisement_raffle_t *raffle_packet_get()
{
  m_raffle_packet.unlock_flags = state_unlock_get();
  m_raffle_packet.adv_flags = m_adv_flags;
  #ifdef CONFIG_BADGE_TYPE_MASTER
  // TODO: reenable master override
    //m_raffle_packet.adv_flags |= RAFFLE_ADV_GLITCH;
  #endif
  #ifdef CONFIG_BADGE_TYPE_TROLL
  m_raffle_packet.adv_flags |= RAFFLE_ADV_GLITCH;
  #endif
  m_raffle_packet.badge_type = BADGE_TYPE;
  m_raffle_packet.random = esp_random()%65535;
  *(uint16_t *)m_raffle_packet.crc = crc16_le(0, (uint8_t *)&m_raffle_packet, sizeof(ble_advertisement_raffle_t) - 2);
  return &m_raffle_packet;
}

void raffle_adv_temp(uint8_t flags){
  m_adv_update = flags;
}
/**
 * @brief Initialize the botnet
 */
void raffle_init()
{
  m_raffle_packet.serial = util_serial_get();
  m_payload_queue = xQueueCreate(1, sizeof(raffle_adv_flags_t));
  // static StaticTask_t botnet;

  xTaskCreate(__payload_task, "Botnet Payloads", 6000, NULL, TASK_PRIORITY_MEDIUM, NULL);
  xTaskCreate(__adv_task, "Advertisement updater", 6000, NULL, TASK_PRIORITY_MEDIUM, NULL);

  p_flags = state_adv_flags_get();
  m_adv_flags = *p_flags;
  *(uint16_t *)m_raffle_packet.crc = crc16_le(
      0, (uint8_t *)&m_raffle_packet, sizeof(ble_advertisement_raffle_t) - 2);
}


// /**
//  * @brief Level up the player
//  */
// void botnet_level_up() {
//   botnet_state_t* p_state = state_botnet_get();
//   p_state->level = (p_state->level + 1) % BOTNET_LEVEL_MAX;
//   ESP_LOGD(TAG, "Botnet level up, new level %d", p_state->level);
// }

/**
 * @brief : Set a botnet packet. This includes setting the CRC
 * @param p_packet : Pointer to botnet packet to set
 */
// void raffle_packet_set(ble_advertisement_raffle_t *p_packet)
// {
//   memcpy(&m_raffle_packet, p_packet, sizeof(ble_advertisement_raffle_t));
//   *(uint16_t *)m_raffle_packet.crc =
//       crc16_le(0, (uint8_t *)p_packet, sizeof(ble_advertisement_raffle_t) - 2);
//   //m_timestamp = *(uint32_t *)(p_packet->timestamp);
//   ESP_LOGD(TAG,
//            "Setting raffle packet. CRC=0x%02x Timestamp=%d Current Time=%d",
//            (uint16_t)*m_raffle_packet.crc, m_timestamp, time_manager_now_sec());
// }

/**
 * @brief Validate a given raffle packet
 * @param p_packet : Pointer to the packet to validate
 * @return True if valid
 */
bool raffle_payload_validate(ble_advertisement_raffle_t *p_packet)
{
  uint16_t crc =
      crc16_le(0, (uint8_t *)p_packet, sizeof(ble_advertisement_raffle_t) - 2);
  uint16_t packet_crc = *(uint16_t *)p_packet->crc;
  ESP_LOGD(TAG, "Expecting 0x%02x --- Got 0x%02x", packet_crc, crc);
  return (packet_crc == crc);
}

/**
 * @brief Reset botnet state, careful player will lose all progress!
 */
void botnet_reset()
{
  ESP_LOGD(TAG, "%s", __func__);
  // botnet_state_t* p_state = state_botnet_get();
  // p_state->level = BOTNET_DEFAULT_LEVEL;
  // p_state->points = BOTNET_DEFAULT_POINTS;
  // p_state->botnet_id = util_serial_get() & 0xffff;
  // p_state->experience = BOTNET_DEFAULT_EXPERIENCE;
  // p_state->first_run = true;
}
