#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>

#include <tsetlin.pb-c.h>
#include "clause.h"

uint8_t* tsetlin_read_file(const char* path, size_t* out_size);

void tsetlin_step(Tsetlin* model, uint8_t* X_img, int8_t y_target, uint32_t T, float s);

int tsetlin_evaluate(Tsetlin* model, uint8_t* input, int32_t *out_votes, uint8_t* out_class);
