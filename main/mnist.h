#include <sys/unistd.h>
#include <esp_vfs_fat.h>

uint32_t mnist_image_info(const char* path, int* out_rows, int* out_cols);
uint8_t* mnist_load_image(FILE* f, int idx, int rows, int cols);

uint32_t mnist_label_info(const char* path);
int8_t mnist_load_label(FILE* f, int idx);

void mnist_print_img(const uint8_t* buf);
