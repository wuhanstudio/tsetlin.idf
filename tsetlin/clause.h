
#include <sys/unistd.h>
#include <esp_random.h>

#include "tsetlin.pb-c.h"

float random_float_01(void);

uint8_t clause_evaluate(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature);

void clause_update_type_I(ClauseCompressed* clause, uint8_t* input, int8_t clause_output, uint32_t n_state, uint32_t n_feature, float s);
void clause_update_type_II(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature);