
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/unistd.h>
#endif

#include <tsetlin.pb-c.h>

#if defined(ESP_PLATFORM)
  /* ESP-IDF */
  #include <esp_random.h>
#elif defined(__ZEPHYR__)
  /* Zephyr RTOS */
  #include <zephyr/random/random.h>
#elif defined(__RTTHREAD__)
  /* RT-Thread RTOS */
  #include <rtthread.h>
  #include <rtdevice.h>
  #include <fast_rand.h>
#else
  /* POSIX */
  #include <fast_rand.h>
#endif

float random_float_01(void);

uint8_t clause_evaluate(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature);

void clause_update_type_I(ClauseCompressed* clause, uint8_t* input, int8_t clause_output, uint32_t n_state, uint32_t n_feature, float s);
void clause_update_type_II(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature);