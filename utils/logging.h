#pragma once
#include <stdarg.h>

#if defined(ESP_PLATFORM)
    /* ================= ESP-IDF ================= */
    #include "esp_log.h"

    #define LOGE(tag, fmt, ...) ESP_LOGE(tag, fmt, ##__VA_ARGS__)
    #define LOGW(tag, fmt, ...) ESP_LOGW(tag, fmt, ##__VA_ARGS__)
    #define LOGI(tag, fmt, ...) ESP_LOGI(tag, fmt, ##__VA_ARGS__)
    #define LOGD(tag, fmt, ...) ESP_LOGD(tag, fmt, ##__VA_ARGS__)

#elif defined(__ZEPHYR__)
    /* ================= Zephyr ================= */
    #include <zephyr/logging/log.h>

    #define LOG_TAG my_module   /* override per file if needed */

    LOG_MODULE_REGISTER(LOG_TAG, LOG_LEVEL_INF);

    #define LOGE(tag, fmt, ...) LOG_ERR("[%s] " fmt, tag, ##__VA_ARGS__)
    #define LOGW(tag, fmt, ...) LOG_WRN("[%s] " fmt, tag, ##__VA_ARGS__)
    #define LOGI(tag, fmt, ...) LOG_INF("[%s] " fmt, tag, ##__VA_ARGS__)
    #define LOGD(tag, fmt, ...) LOG_DBG("[%s] " fmt, tag, ##__VA_ARGS__)

#elif defined(__RTTHREAD__)
    /* ================= RT-Thread ================= */
    #include <rtthread.h>

    #define LOGE(tag, fmt, ...) rt_kprintf("[E][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LOGW(tag, fmt, ...) rt_kprintf("[W][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LOGI(tag, fmt, ...) rt_kprintf("[I][%s] " fmt "\n", tag, ##__VA_ARGS__)
    #define LOGD(tag, fmt, ...) rt_kprintf("[D][%s] " fmt "\n", tag, ##__VA_ARGS__)
#endif
