#include <sdmmc_cmd.h>

#include "board.h"

#define MOUNT_POINT "/sdcard"

sdmmc_card_t* sdcard_init();
void sdcard_deinit(sdmmc_card_t* card);
