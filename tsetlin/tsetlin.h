#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/unistd.h>
#endif

#include <tsetlin.pb-c.h>
#include <logging.h>
#include "clause.h"

uint8_t* tsetlin_read_file(const char* path, size_t* out_size);

void tsetlin_step(Tsetlin* model, uint8_t* X_img, int8_t y_target, uint32_t T, float s);

int tsetlin_evaluate(Tsetlin* model, uint8_t* input, int32_t *out_votes, uint8_t* out_class);
