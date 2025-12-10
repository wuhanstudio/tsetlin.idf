#include <sys/unistd.h>

#include "tsetlin.pb-c.h"

uint8_t* tsetlin_read_file(const char* path, size_t* out_size);

int tsetlin_evaluate(Tsetlin* model, uint8_t* input, int32_t *out_votes, uint8_t* out_class);
