#include <sdmmc_cmd.h>

#define MOUNT_POINT "/sdcard"

// Choose SD mode: SDIO or SPI
// #define USE_SPI_MODE 1
#define USE_SDIO_MODE 1

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define PIN_NUM_MISO  19
#define PIN_NUM_MOSI  23
#define PIN_NUM_CLK   18
#define PIN_NUM_CS    5

#define PIN_SDIO_CLK 39
#define PIN_SDIO_CMD 38
#define PIN_SDIO_D0  40

sdmmc_card_t* sdcard_init();
void sdcard_deinit(sdmmc_card_t* card);
