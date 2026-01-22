#include "sdcard.h"

/* mounting info */
static FATFS fat_fs;
static struct fs_mount_t mp = {
    .type = FS_FATFS,
    .fs_data = &fat_fs,
};

int sdcard_init() {
    const char *disk_mount_pt = DISK_MOUNT_PT;
    mp.mnt_point = disk_mount_pt;

    return fs_mount(&mp);   
}

int sdcard_deinit() {
    return fs_unmount(&mp);
}