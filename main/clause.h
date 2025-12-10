
#include <sys/unistd.h>

#include "tsetlin.pb-c.h"

uint8_t clause_evaluate(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature);
