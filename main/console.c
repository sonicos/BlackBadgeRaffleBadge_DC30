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

const static char *TAG = "RAFFLE::Console";
static TaskHandle_t m_console_task_handle = NULL;


#ifdef CONFIG_BADGE_TYPE_MASTER
static int __fs(int argc, char **argv)
{
    DIR *d;
    struct dirent *dir;
    d = opendir("/spiflash");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s ", dir->d_name);
            char fd[128];
            sprintf(fd, "/spiflash/%s", dir->d_name);
            struct stat *buf;

            buf = malloc(sizeof(struct stat));

            stat(fd, buf);
            int size = buf->st_size;
            printf("%d \n", size);

            free(buf);
        }
        closedir(d);
    }
    return 0;
}
void register_fs()
{
    const esp_console_cmd_t cmd = {
        .command = "ls",
        .help = "Get contents of flash",
        .hint = NULL,
        .func = &__fs,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
static int __get_drawings(int argc, char **argv){
    drawings_dump();
    return 0;
}
void register_drawings()
{
    const esp_console_cmd_t cmd = {
        .command = "drawings",
        .help = "Dump drawings",
        .hint = NULL,
        .func = &__get_drawings,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int __get_peers(int argc, char **argv){
    peers_dump();
    return 0;
}
void register_peers()
{
    const esp_console_cmd_t cmd = {
        .command = "peers",
        .help = "Dump peers",
        .hint = NULL,
        .func = &__get_peers,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


static int __raffle_start(int argc, char **argv){
    drawing_start();
    return 0;
}
void register_raffle_start()
{
    const esp_console_cmd_t cmd = {
        .command = "raffle_start",
        .help = "Start advertising raffle",
        .hint = NULL,
        .func = &__raffle_start,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int __mesh_reset(int argc, char **argv){
    mesh_master_reset();
    printf("Mesh has been reset!");
    return 0;
}

void register_mesh_reset()
{
    const esp_console_cmd_t cmd = {
        .command = "mesh_reset",
        .help = "Reset the mesh",
        .hint = NULL,
        .func = &__mesh_reset,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int __raffle_winner(int argc, char **argv){
    drawing_select_winner();
    return 0;
}

void register_raffle_winner()
{
    const esp_console_cmd_t cmd = {
        .command = "raffle_winner",
        .help = "Choose winner",
        .hint = NULL,
        .func = &__raffle_winner,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


#endif


static int unlock_mud(int argc, char **argv){
    if(!state_is_unlocked(UNLOCK_CODE_MUD)) state_unlock(UNLOCK_CODE_MUD);
    printf("Congratulations! You unlocked the MUD flag\r\n");
    return 0;
}

static int unlock_console(int argc, char **argv){
    if(!state_is_unlocked(UNLOCK_CONSOLE)) state_unlock(UNLOCK_CONSOLE);
    printf(";)\r\n");
    return 0;
}

static int unlock_ateam(int argc, char **argv){
    if(!state_is_unlocked(UNLOCK_ATEAM)) state_unlock(UNLOCK_ATEAM);
    printf("Congratulations! You unlocked the A-Team Flag\r\n");
    return 0;
}

static int show_flags(int argc, char **argv){
    
    printf("Beacon %s: %s - %s\r\n", state_is_unlocked(UNLOCK_BEACON_A)? "Scav Hunt": "xxxxxxxx", state_is_unlocked(UNLOCK_BEACON_A)?"Unlocked":"Locked");
    printf("Beacon %s: %s - %s\r\n", state_is_unlocked(UNLOCK_BEACON_B)? "Packet Hacking": "xxxxxxxx", state_is_unlocked(UNLOCK_BEACON_B)?"Unlocked":"Locked");
    printf("Beacon %s: %s - %s\r\n", state_is_unlocked(UNLOCK_BEACON_B)? "SE Village": "xxxxxxxx", state_is_unlocked(UNLOCK_BEACON_C)?"Unlocked":"Locked");
    printf("Beacon %s: %s - %s\r\n", state_is_unlocked(UNLOCK_BEACON_D)? "Crypto and Privacy": "xxxxxxxx", state_is_unlocked(UNLOCK_BEACON_D)?"Unlocked":"Locked");
    printf("Beacon %s: %s - %s\r\n", state_is_unlocked(UNLOCK_BEACON_E)? "Hardware Hacking": "xxxxxxxx", state_is_unlocked(UNLOCK_BEACON_E)?"Unlocked":"Locked");
    printf("Captured %s: %s - %s\r\n", state_is_unlocked(UNLOCK_TROLL_INNOC)? "Troll": "xxxxxxxx", state_is_unlocked(UNLOCK_TROLL_INNOC)?"Unlocked":"Locked");
    printf("Captured %s: %s - %s\r\n", state_is_unlocked(UNLOCK_MASTER_INNOC)? "Troll": "xxxxxxxx", state_is_unlocked(UNLOCK_MASTER_INNOC)?"Unlocked":"Locked");
    printf("Code %s: %s - %s\r\n", state_is_unlocked(UNLOCK_CODE_MUD)? "MUD": "xxxxxxxx", state_is_unlocked(UNLOCK_CODE_MUD)?"Unlocked":"Locked");
    printf("Code %s: %s - %s\r\n", state_is_unlocked(UNLOCK_ATEAM)? "Mr T.": "xxxxxxxx", state_is_unlocked(UNLOCK_ATEAM)?"Unlocked":"Locked");

    return 0;

}

static int get_version(int argc, char **argv)
{
    const char *model;
    esp_chip_info_t info;
    esp_chip_info(&info);

    switch (info.model)
    {
    case CHIP_ESP32:
        model = "ESP32";
        break;
    case CHIP_ESP32S2:
        model = "ESP32-S2";
        break;
    case CHIP_ESP32S3:
        model = "ESP32-S3";
        break;
    case CHIP_ESP32C3:
        model = "ESP32-C3";
        break;
    case CHIP_ESP32H2:
        model = "ESP32-H2";
        break;
    default:
        model = "Unknown";
        break;
    }
    printf("DEF CON 30 Black Badge Raffle\r\nBadge Version:%s\r\n", VERSION);
    //printf("IDF Version:%s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", model);
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);
    printf("Serial Number:%08x\r\n", util_serial_get());
    return 0;
}

static void register_friend(void)
{
    const esp_console_cmd_t cmd = {
        .command = "friend",
        .help = NULL,
        .hint = NULL,
        .func = &unlock_console,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static void register_version(void)
{
    const esp_console_cmd_t cmd = {
        .command = "ver",
        .help = NULL,
        .hint = NULL,
        .func = &get_version,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
static void register_ateama(void)
{
    const esp_console_cmd_t cmd = {
        .command = "5556162",
        .help = NULL,
        .hint = NULL,
        .func = &unlock_ateam,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
static void register_ateamb(void)
{
    const esp_console_cmd_t cmd = {
        .command = "555-6162",
        .help = NULL,
        .hint = NULL,
        .func = &unlock_ateam,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}


static void register_mud(void)
{
    const esp_console_cmd_t cmd = {
        .command = "RFC1149",
        .help = NULL,
        .hint = NULL,
        .func = &unlock_mud,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static void register_flags(void)
{
    const esp_console_cmd_t cmd = {
        .command = "flags",
        .help = "Display the unlocked flags",
        .hint = NULL,
        .func = &show_flags,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
/** 'restart' command restarts the program */

static int restart(int argc, char **argv)
{
    //ESP_LOGI(TAG, "Restarting");
    printf("Reboot triggered. Standby");
    reboot_trigger();
    return 0;
}

static void register_restart(void)
{
    const esp_console_cmd_t cmd = {
        .command = "restart",
        .help = NULL,
        .hint = NULL,
        .func = &restart,
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static void initialize_console()
{

    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));

    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Disable buffering on stdin and stdout */
    // setvbuf(stdin, NULL, _IONBF, 0);
    // setvbuf(stdout, NULL, _IONBF, 0);

    /* Enable non-blocking mode on stdin and stdout */
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

    // Thanks internets!
    // https://github.com/espressif/esp-idf/issues/8789#issuecomment-1106216207
    esp_vfs_dev_usb_serial_jtag_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    esp_vfs_dev_usb_serial_jtag_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
    usb_serial_jtag_driver_config_t cfg;
    cfg.tx_buffer_size = 1024;
    cfg.rx_buffer_size = 1024;
    esp_err_t ret = ESP_OK;
    usb_serial_jtag_driver_install(&cfg);

    esp_vfs_usb_serial_jtag_use_driver();

    /* Initialize the console */
    esp_console_config_t console_config = {
        .max_cmdline_args = 8,
        .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
        .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback *)&esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(CONSOLE_HISTORY_LEN);

#ifdef CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(CONSOLE_HISTORY_PATH);
#endif

    ESP_LOGI(TAG, "Console started");
}

static void __console_task(void *parameters)
{
    /*
           #if CONFIG_STORE_HISTORY
           initialize_filesystem();
           #endif
           */

    initialize_console();

    /* Register commands */
    esp_console_register_help_command();
    register_friend();

    #ifdef CONFIG_BADGE_TYPE_MASTER
    register_peers();
    register_drawings();
    register_raffle_start();
    register_raffle_winner();
    register_mesh_reset();
    register_fs();
    #endif
    // register_system();

    /* Prompt to be printed before each line. This can be customized, made
     * dynamic, etc. */
    
    const char *prompt = LOG_COLOR_I PROMPT LOG_RESET_COLOR;

    while (!state_is_unlocked(UNLOCK_CONSOLE))
    {
        // Get a line using linenoise. The line is returned when ENTER is pressed.
        char *line = linenoise(">");
        if (line == NULL || strlen(line) == 0)
        { /* Ignore empty lines */
            linenoiseFree(line);
            continue;
        }


        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND)
        {
            printf("I don't know you...\r\n");
        }
        else if (err == ESP_OK && ret != ESP_OK)
        {
            printf("Command returned non-zero error code: 0x%x\n\n", ret);
        }
        else if (err != ESP_OK)
        {
            printf("Internal error: 0x%x\n\n", err);
        }
        /* linenoise allocates line buffer on the heap, so need to free it */

        linenoiseFree(line);
    }

    if (linenoiseProbe())
    {
        // didnt detect connection at init
        linenoise(">"); // wait for someone to hit enter
    }
    register_ateama();
    register_ateamb();
    register_mud();
    //register_flags();
    register_restart();
    // while (true) {

          printf(
      "\n"
      "Welcome to the Black Badge Raffle Badge Challenge\n\n"
      "Congratulations on obtaining this badge. There are some \n"
      "flags that you can unlock if you follow the hints and explore the con.\n\n"
      "I wish you luck, and I thank you for playing along with me.\n"
      "You can type 'help' for a list of commands (of which there aren't many).\n"
      "You can also try to enter codes at the command prompt. If they dont work\n"
      "you may have to leverage the hotline. 833-BLK-BDGE. Good Luck!\n\n"
      "With all sincerity, thank you for playing along.\n\n"
      "-Sonicos\n\n");

    //   DELAY(1000);
    //   taskYIELD();

    // }


    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status)
    { /* zero indicates success */
        printf(
            "\n"
            "Your terminal application does not support escape sequences.\n"
            "Line editing and history features are disabled.\n"
            "Use PuTTY instead!\n");
        linenoiseSetDumbMode(1);

#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences, don't use color
         * codes in the prompt. */
        prompt = PROMPT;
#endif // CONFIG_LOG_COLORS
    }

    // Main loop runs forever
    while (true)
    {
        // Get a line using linenoise. The line is returned when ENTER is pressed.
        char *line = linenoise(prompt);
        if (line == NULL || strlen(line) == 0)
        { /* Ignore empty lines */
            linenoiseFree(line);
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);

#ifdef CONFIG_STORE_HISTORY
        /* Save command history to filesystem */
        linenoiseHistorySave(CONSOLE_HISTORY_PATH);
#endif

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND)
        {
            printf("LAWL. No.\n\n");
        }
        else if (err == ESP_OK && ret != ESP_OK)
        {
            printf("Command returned non-zero error code: 0x%x\n\n", ret);
        }
        else if (err != ESP_OK)
        {
            printf("Internal error: 0x%x\n\n", err);
        }
        /* linenoise allocates line buffer on the heap, so need to free it */

        linenoiseFree(line);
    }

    // Just in case in breaks free of the while()
    vTaskDelete(NULL);
}

/**
 * @brief Pause the console task
 */
void console_task_pause()
{
    if (m_console_task_handle != NULL)
    {
        ESP_LOGD(TAG, "Pausing console");
        vTaskSuspend(m_console_task_handle);
    }
}

/**
 * @brief Resume the console task
 */
void console_task_resume()
{
    if (m_console_task_handle != NULL)
    {
        ESP_LOGD(TAG, "Resuming Console");
        vTaskResume(m_console_task_handle);
    }
}

void console_task_start()
{
    ESP_LOGI(TAG, "Starting Console");
    m_console_task_handle = xTaskCreate(
        __console_task, "Console", 8000, NULL, TASK_PRIORITY_CONSOLE, NULL);
}