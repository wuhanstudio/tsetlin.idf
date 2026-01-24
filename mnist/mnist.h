#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/unistd.h>
#endif

#include <stdint.h>
#include <logging.h>

uint32_t mnist_image_info(const char* path, int* out_rows, int* out_cols);
uint8_t* mnist_load_image(FILE* f, int idx, int rows, int cols);
uint8_t* mnist_load_next_image(FILE* f, int idx, int rows, int cols);

uint32_t mnist_label_info(const char* path);
int8_t mnist_load_label(FILE* f, int idx);
int8_t mnist_load_next_label(FILE* f, int idx);

void mnist_print_img(const uint8_t* buf);

// Static functions used internally
// float* mnist_int_to_float(uint8_t *src, int rows, int cols);
// void misst_normalize_img(float *X, int rows, int cols);

// int mnist_booleanize_n_bit(float x, int num_bits, uint8_t *out_bits);
// uint8_t* mnist_booleanize_features(
//     float* X,
//     int rows,
//     int cols,
//     int num_bits
// );

uint8_t* mnist_booleanize_img_n_bit(
    uint8_t* img,
    int rows,
    int cols,
    int num_bits
);

void mnist_booleanize_img(uint8_t* img, uint32_t size, uint8_t threshold);