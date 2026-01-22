#ifndef __SDCARD_H__
#define __SDCARD_H__

#if defined(CONFIG_FAT_FILESYSTEM_ELM)
    #define FS_RET_OK FR_OK
#else
    #define FS_RET_OK 0
#endif

#include <ff.h>

#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"

int sdcard_init();

#endif /* __SDCARD_H__ */
