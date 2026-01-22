
// Choose your board using menuconfig (j / k to navigate, Enter to select):
// idf.py menuconfig

// Choose SD mode: SPI or SDIO
#if defined (CONFIG_ESP32F_BOARD)
    #define USE_SPI_MODE 1
#endif

#if defined (CONFIG_ESP32_CAM_BOARD) || defined (CONFIG_ESP32_S3_CAM_BOARD)
    #define USE_SDIO_MODE 1
#endif

#if defined (CONFIG_ESP32_CAM_BOARD) 
    #define SDMMC_BUS_WIDTH_4
#endif

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#if defined (CONFIG_ESP32F_BOARD)
    #define PIN_NUM_MISO  19
    #define PIN_NUM_MOSI  23
    #define PIN_NUM_CLK   18
    #define PIN_NUM_CS    5
#endif

#if defined (CONFIG_ESP32_CAM_BOARD)
    #define PIN_SDIO_CLK 14
    #define PIN_SDIO_CMD 15
    #define PIN_SDIO_D0  2
    #define PIN_SDIO_D1  4
    #define PIN_SDIO_D2  12
    #define PIN_SDIO_D3  13
#endif

#if defined (CONFIG_ESP32_S3_CAM_BOARD)
    #define PIN_SDIO_CLK 39
    #define PIN_SDIO_CMD 38
    #define PIN_SDIO_D0  40
#endif
