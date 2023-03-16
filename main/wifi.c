
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
#if CONFIG_BBR_ENABLE_WIFI
/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* esp netif object representing the WIFI station */
static esp_netif_t *sta_netif = NULL;

uint8_t ca_pem_start[] asm("_binary_ca_pem_start");
uint8_t ca_pem_end[]   asm("_binary_ca_pem_end");
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static const char *TAG = "RAFFLE:WiFi";

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        #ifdef CONFIG_BADGE_TYPE_BEACON
        led_pulse_configure(3000,30,EYE_RED);
        led_eye_pulse_start();
        #endif
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        #ifdef CONFIG_BADGE_TYPE_BEACON
        led_pulse_configure(1000,30,EYE_RED);
        led_eye_pulse_start();
        #endif
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        #ifdef CONFIG_BADGE_TYPE_BEACON
        led_eye_pulse_stop();
        #endif
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

static void initialise_wifi(void)
{
    

    unsigned int ca_pem_bytes = ca_pem_end - ca_pem_start;


    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
#ifdef CONFIG_WIFI_TARGET_HOME
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "GoTheFuckOn",
            .password = "BestPassword3V4R",
            //.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,

        },
    };
#endif

#ifdef CONFIG_WIFI_TARGET_DEFCON
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "DefCon",

        },
    };
#endif
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    

#ifdef CONFIG_WIFI_TARGET_DEFCON
    
    // cert validate
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_ca_cert(ca_pem_start, ca_pem_bytes) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_EAP_ID, strlen(WIFI_EAP_ID)) );
    //ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)) );
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD)) );
    // ESP_ERROR_CHECK(esp_wifi_sta_wpa2_use_default_cert_bundle(true));
    ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
#endif
    ESP_ERROR_CHECK( esp_wifi_start() );
    
}

static void __sntp_cb(struct timeval *tv){
    ESP_LOGI(TAG, "S: %u U: %llu", tv->tv_sec, tv-> tv_usec);
    time_manager_ntpupdate(tv);
    #ifdef CONFIG_BADGE_TYPE_BEACON
        led_pulse_configure(1000,30,EYE_GREEN);
        led_eye_pulse_start();
        DELAY(3000);
        led_eye_pulse_stop();
    #endif
}

static void __wifi_task(void *pvParameters)
{
    esp_netif_ip_info_t ip;
    memset(&ip, 0, sizeof(esp_netif_ip_info_t));
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        if (esp_netif_get_ip_info(sta_netif, &ip) == 0) {
            ESP_LOGI(TAG, "~~~~~~~~~~~");
            ESP_LOGI(TAG, "IP:"IPSTR, IP2STR(&ip.ip));
            ESP_LOGI(TAG, "MASK:"IPSTR, IP2STR(&ip.netmask));
            ESP_LOGI(TAG, "GW:"IPSTR, IP2STR(&ip.gw));
            ESP_LOGI(TAG, "~~~~~~~~~~~");
        }
    }
}

void wifi_enable()
{
    //ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    //xTaskCreate(&__wifi_task, "WiFi Task", 4096, NULL, 5, NULL);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(__sntp_cb);
    sntp_init();
}
#endif