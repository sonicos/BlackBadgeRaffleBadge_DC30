idf_component_register(SRCS "reboot.c" "console.c" "mesh.c" "raffle.c" "main.c" "boot.c" "led.c" "btn.c" "time_manager.c" "ble.c" "time_manager.c" "peers.c"  "util.c" "state.c" "sd.c" "ui.c" "drawing.c" "wifi.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES ca.crt
                    
                      )
if("${CONFIG_NUKE_FATFS}")
  fatfs_create_spiflash_image(vfs ../spiflash FLASH_IN_PROJECT)
endif()
