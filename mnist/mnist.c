#include "mnist.h"

static const char *TAG = "mnist";

#define MNIST_X_MEAN 33.318f
#define MNIST_X_STD 78.567f

static inline float norm_cdf(float x) {
    return 0.5f * (1.0f + erff(x / 1.41421356237f)); // sqrt(2)
}

static uint32_t read_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
           (uint32_t)p[3];
}

static float* mnist_int_to_float(uint8_t *src, int rows, int cols) {
    float *dst = malloc(rows * cols * sizeof(float));

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            dst[i * cols + j] = (float) src[i * cols + j];
        }
    }
    return dst;
}

static void misst_normalize_img(float* X, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            X[i * cols + j] = (X[i * cols + j] - MNIST_X_MEAN) / MNIST_X_STD;
            X[i * cols + j] = norm_cdf(X[i * cols + j]);
        }
    }
}

static int mnist_booleanize_n_bit(float x, int num_bits, uint8_t *out_bits) {
    if (x < 0.0f || x > 1.0f)
        return -1;

    if (!(num_bits == 1 || num_bits == 2 || num_bits == 4 || num_bits == 8))
        return -2;

    int max_val = (1 << num_bits) - 1;

    /* round-to-nearest-even */
    int int_val = (int) lrintf(x * max_val);

    for (int i = 0; i < num_bits; i++) {
        out_bits[i] = (int_val >> (num_bits - 1 - i)) & 1;
    }

    return 0;
}

static uint8_t* mnist_booleanize_features(
    float* X,
    int rows,
    int cols,
    int num_bits
) {
    int out_cols = cols * num_bits;

    uint8_t* X_bool = malloc(rows * out_cols * sizeof(uint8_t));

    int offset = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            mnist_booleanize_n_bit(X[i * cols + j], num_bits, &X_bool[offset]);
            offset += num_bits;
        }
    }
    return X_bool;
}

uint8_t* mnist_booleanize_img_n_bit(
    uint8_t* img,
    int rows,
    int cols,
    int num_bits
) {
    // Booleanize image using 8-bit representation
    float* float_img = mnist_int_to_float(img, rows, cols);
    misst_normalize_img(float_img, rows, cols);

    uint8_t* bool_img = mnist_booleanize_features(float_img, rows, cols, num_bits);
    free(float_img);

    return bool_img;
}

void mnist_booleanize_img(uint8_t* img, uint32_t size, uint8_t threshold) {
    for (uint32_t i = 0; i < size; i++) {
        img[i] = (img[i] > threshold) ? 1 : 0;
    }
}

uint32_t mnist_image_info(const char* path, int* out_rows, int* out_cols) {
    FILE* f = fopen(path, "r");
    if (!f) {
        LOGE(TAG, "Failed to open file %s", path);
        return 0;
    }

    uint8_t header[16];
    if (fread(header, 1, 16, f) != 16) { 
        fclose(f); 
        LOGE(TAG, "Failed to read header from file %s", path);
        return 0;
    }

    uint32_t magic      = read_u32_be(&header[0]);
    uint32_t num_images = read_u32_be(&header[4]);
    uint32_t rows       = read_u32_be(&header[8]);
    uint32_t cols       = read_u32_be(&header[12]);

    if (magic != 0x00000803) { 
        fclose(f); 
        LOGE(TAG, "Invalid magic number in file %s", path);
        return 0;
    }

    *out_rows = (int)rows;
    *out_cols = (int)cols;

    fclose(f);

    return num_images;
}

uint8_t* mnist_load_image(FILE* f, int idx, int rows, int cols) {
    size_t total = (size_t)rows * cols;
    uint8_t* buf = (uint8_t*)malloc(total);
    if (!buf) { fclose(f); return NULL; }

    fseek(f, 16 + (size_t)idx * total, SEEK_SET);

    if (fread(buf, 1, total, f) != total) { free(buf); fclose(f); return NULL; }

    return buf;
}

uint8_t* mnist_load_next_image(FILE* f, int idx, int rows, int cols) {
    size_t total = (size_t)rows * cols;
    uint8_t* buf = (uint8_t*)malloc(total);
    if (!buf) { fclose(f); return NULL; }

    if (fread(buf, 1, total, f) != total) { free(buf); fclose(f); return NULL; }

    return buf;
}

uint32_t mnist_label_info(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        LOGE(TAG, "Failed to open file %s", path);
        return 0;
    }

    uint8_t header[8];
    if (fread(header, 1, 8, f) != 8) { 
        fclose(f); 
        LOGE(TAG, "Failed to read header from file %s", path);
        return 0; 
    }

    uint32_t magic      = read_u32_be(&header[0]);
    uint32_t num_labels = read_u32_be(&header[4]);

    if (magic != 0x00000801) { 
        fclose(f); 
        LOGE(TAG, "Invalid magic number in file %s", path);
        return 0; 
    }

    fclose(f);

    return num_labels;
}

int8_t mnist_load_label(FILE* f, int idx) {
    fseek(f, 8 + (size_t)idx, SEEK_SET);

    uint8_t label;
    if (fread(&label, 1, 1, f) != 1) { fclose(f); return 0; }

    return label;
}

int8_t mnist_load_next_label(FILE* f, int idx) {
    uint8_t label;
    if (fread(&label, 1, 1, f) != 1) { fclose(f); return 0; }

    return label;
}

// ASCII lib from (https://www.jianshu.com/p/1f58a0ebf5d9)
static const char codeLib[] = "@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.   ";
void mnist_print_img(const uint8_t* buf)
{
    for(int y = 0; y < 28; y++) 
    {
        for (int x = 0; x < 28; x++) 
        {
            int index = 0; 
            if(buf[y*28+x] > 75) index =69;
            if(index < 0) index = 0;
            printf("%c",codeLib[index]);
            printf("%c",codeLib[index]);
        }
        printf("\n");
    }
}
