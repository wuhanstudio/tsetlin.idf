#include "sdcard.h"

/* mounting info */
static FATFS fat_fs;
static struct fs_mount_t mp = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
};

static const char *disk_mount_pt = DISK_MOUNT_PT;

int sdcard_init() {
    mp.mnt_point = disk_mount_pt;

    return fs_mount(&mp);   
}
