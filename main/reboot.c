
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
const static char *TAG = "RAFFLE::Reboot";

static bool m_reboot_prevent_menu = false;
static bool m_reboot_prevent_mesh = false;
static bool m_reboot_prevent_save = false;
static bool m_reboot_prevent_console = false;
static bool m_reboot_master_force = false;
static bool m_reboot_triggered = false;


void __safe_reboot(){
    drv_sd_unmount();
    DELAY(1000);
    esp_restart();
}
static void IRAM_ATTR __reboot_task(void *parameters)
{
    /* I cannot describe in words how much I hate having to do this
    If someone figures out out to solve the damn memory leak issue that arises
    from BLE rotation, let me know and I'll buy you a beer. This hacky reboot
    shit sucks :(
    */
    uint32_t timer = 0;
    while (1)
    {
        if (m_reboot_master_force)
        {
            // OH SHIT SON
            if (timer <= REBOOT_TIME_MASTER_IGNORE)
            {
                // uptime is safe for my purposes, so you'll survive... this time
                m_reboot_master_force = false;
            }
            else if (!(m_reboot_prevent_console || m_reboot_prevent_menu || m_reboot_prevent_mesh || m_reboot_prevent_save))
            {
                // No Mr. Bond, I expect you to die.
                __safe_reboot();;
            }
        }
        if (m_reboot_triggered && !(m_reboot_prevent_console || m_reboot_prevent_menu || m_reboot_prevent_mesh || m_reboot_prevent_save))
        {
            // goodnight sweet prince
            __safe_reboot();
        }
        DELAY(REBOOT_TIME_INCREMENT);
        timer += REBOOT_TIME_INCREMENT;
    }
    vTaskDelete(NULL); // in case the impossible is possible
}

void reboot_prevent(reboot_mode_t mode)
{
    switch (mode)
    {
    case reboot_menu:
        m_reboot_prevent_menu = true;
        break;
    case reboot_mesh:
        m_reboot_prevent_mesh = true;
        break;
    case reboot_save:
        m_reboot_prevent_save = true;
        break;
    case reboot_console:
        m_reboot_prevent_console = true;
        break;
    default:
        break;
    }
}

void reboot_enable(reboot_mode_t mode)
{
    switch (mode)
    {
    case reboot_menu:
        m_reboot_prevent_menu = false;
        break;
    case reboot_mesh:
        m_reboot_prevent_mesh = false;
        break;
    case reboot_save:
        m_reboot_prevent_save = false;
        break;
    case reboot_console:
        m_reboot_prevent_console = false;
        break;
    default:
        break;
    }
}

void reboot_master_override()
{
    m_reboot_master_force = true;
}

void reboot_trigger()
{
    m_reboot_triggered = true;
}

void reboot_terrible_hack_timer_start()
{
    ESP_LOGD(TAG, "Crappy reboot timer hack is now engaged. Please forgive me.");
    xTaskCreate(__reboot_task, "Reboot Task", 4096, NULL, 5, NULL);
}
