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

const static char *TAG = "RAFFLE::Mesh";
static SemaphoreHandle_t m_mutex = NULL;
static StaticSemaphore_t m_mutex_buffer;
static volatile mesh_state_t *p_state;

static volatile ble_advertisement_mesh_t m_mesh_packet = {
    .winner_serial = 0,
    .timestamp_seconds = TIME_DEFAULT,
    .mesh_flags = 0,
    .stratum = TIME_MAX_STRATUM,
};

void mesh_init()
{
    
    ESP_LOGI(TAG, "Initializing Mesh");
    m_mutex = xSemaphoreCreateMutexStatic(&m_mutex_buffer);
    p_state = state_mesh_get();
}

void bbrmesh_mutex_lock(){
  xSemaphoreTake(m_mutex,portMAX_DELAY);
}
void bbrmesh_mutex_unlock(){
  xSemaphoreGive(m_mutex);
}

// ble_advertisement_mesh_t *mesh_packet_get()
// {
//     //bbrmesh_mutex_lock();
//     m_mesh_packet.winner_serial = p_state->winner_serial;
//     m_mesh_packet.timestamp_seconds = p_state->timestamp_seconds;
//     m_mesh_packet.mesh_flags = p_state->mesh_flags;
//     #ifdef CONFIG_BADGE_TYPE_MASTER
//     m_mesh_packet.stratum = 0;
//     #else
//     m_mesh_packet.stratum = p_state->stratum;
//     #endif
    
//     *(uint16_t *)m_mesh_packet.crc = crc16_le(0, (uint8_t *)&m_mesh_packet, sizeof(ble_advertisement_mesh_t) - 2);
//     //memcpy(&m_raffle_packet, p_packet, sizeof(ble_advertisement_raffle_t));
//   *(uint16_t *)m_raffle_packet.crc = crc16_le(0, (uint8_t *)p_packet, sizeof(ble_advertisement_raffle_t) - 2);
//     //printf("MP: S: %s - T: %u - F: %u - D: %u - C: %4x", m_mesh_packet.winner_serial,m_mesh_packet.timestamp_seconds,m_mesh_packet.mesh_flags,m_mesh_packet.stratum, m_mesh_packet.crc);
//     return &m_mesh_packet;
//     //bbrmesh_mutex_unlock();
// }

void mesh_state_reset()
{
    //ESP_LOGD(TAG, "%s", __func__);
    bbrmesh_mutex_lock();
    mesh_state_t *state = state_mesh_get();
    state->winner_serial = 0;
    state->timestamp_seconds = TIME_DEFAULT;
    state->mesh_flags = 0;
    state->stratum = TIME_MAX_STRATUM;
    bbrmesh_mutex_unlock();
}


void mesh_master_reset(){
    #ifdef CONFIG_BADGE_TYPE_MASTER
    //ESP_LOGD(TAG, "%s", __func__);
    bbrmesh_mutex_lock();
    mesh_state_t *state = state_mesh_get();
    state->winner_serial = 0;
    state->timestamp_seconds = time_manager_now_sec();
    state->mesh_flags = 0;
    state->stratum = 0;
    bbrmesh_mutex_unlock();
    state_save_indicate();
    #endif
}


bool mesh_payload_validate(ble_advertisement_mesh_t *p_packet)
{
    //bbrmesh_mutex_lock();
    uint16_t crc =
        crc16_le(0, (uint8_t *)p_packet, sizeof(ble_advertisement_mesh_t) - 2);
    uint16_t packet_crc = *(uint16_t *)p_packet->crc;
    ESP_LOGD(TAG, "Expecting 0x%02x --- Got 0x%02x", packet_crc, crc);
    
    //bbrmesh_mutex_unlock();
    return (packet_crc == crc);
}


void mesh_advertiser_handler(ble_advertisement_mesh_t *packet)
{
    // Master will ignore this since it contains the defacto standard
    #ifndef CONFIG_BADGE_TYPE_MASTER
    if (mesh_payload_validate(packet))
    {
        bbrmesh_mutex_lock();
        // Determine if the mesh state timestamp is later than us

        if (packet->timestamp_seconds > p_state->timestamp_seconds)
        {
            if (p_state->mesh_flags != packet->mesh_flags){
                
                uint8_t changed_flags = ~(p_state->mesh_flags & packet->mesh_flags);
                p_state->mesh_flags = packet->mesh_flags;
                if (p_state->mesh_flags & MESH_RAFFLE_STARTED) {
                    #ifdef CONFIG_BADGE_TYPE_ENTRANT
                        bling_interrupt_priority(bling_special_raffle);
                    #elif CONFIG_BADGE_TYPE_MASTER
                      // yeah... we're not locking ourselves out

                    #else
                        bling_interrupt_priority(bling_glitch);
                    #endif


                } else if (p_state->mesh_flags & MESH_RAFFLE_ENDED) {
                    bling_cancel_interrupt();
                }
            }

            if (p_state->winner_serial != packet->winner_serial){
                ESP_LOGD(TAG,"Winner changed -> %8x", packet->winner_serial);
                p_state->winner_serial = packet->winner_serial;
                #ifdef CONFIG_BADGE_TYPE_ENTRANT
                if (util_serial_get() == p_state->winner_serial){
                    // WINNER WINNER CHICKEN DINNER
                    printf("Congratulations! You have won the Black Badge Raffle");
                    bling_interrupt(bling_winner,90000, RAFFLE_CAPTURE_NONE);
                    state_unlock(UNLOCK_WINNER);
                } else {
                    bling_interrupt(bling_special_pulse_red,90000, RAFFLE_CAPTURE_NONE);
                }
                #endif
            }

            
            p_state->timestamp_seconds = packet->timestamp_seconds;
            p_state->stratum = packet->stratum + 1;
            if (p_state->stratum > TIME_MAX_STRATUM) p_state->stratum = TIME_MAX_STRATUM;

        }
        else if (packet->timestamp_seconds == p_state->timestamp_seconds)
        {
            if (packet->stratum + 1 < p_state->stratum)
            {
                // update stratum
                p_state->stratum = packet->stratum + 1;
            }
        }
        bbrmesh_mutex_unlock();
    }
    #endif
}



#ifdef CONFIG_BADGE_TYPE_MASTER

void __mesh_update(){
    bbrmesh_mutex_lock();
    p_state->timestamp_seconds = time_manager_now_sec();
    bbrmesh_mutex_unlock();
    state_save_indicate();
}

void mesh_flag_add(mesh_flags_t flag){
    bbrmesh_mutex_lock();
    p_state->mesh_flags |= flag;
    bbrmesh_mutex_unlock();
    __mesh_update();
}

void mesh_flag_remove(mesh_flags_t flag){
    bbrmesh_mutex_lock();
    p_state->mesh_flags &= ~flag;
    bbrmesh_mutex_unlock();
    __mesh_update();
}

void mesh_set_winner(uint32_t serial){
    bbrmesh_mutex_lock();
    p_state->winner_serial=serial;
    
    bbrmesh_mutex_unlock();
    state_save_indicate();
    DELAY(45000);
    mesh_flag_remove(MESH_RAFFLE_STARTED);
    mesh_flag_add(MESH_RAFFLE_ENDED);
}

#endif