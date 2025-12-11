#include "mnist.h"

#include <esp_vfs_fat.h>

static const char *TAG = "mnist";

static uint32_t read_u32_be(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |
           (uint32_t)p[3];
}

void mnist_booleanize_img(uint8_t* img, uint32_t size, uint8_t threshold) {
    for (uint32_t i = 0; i < size; i++) {
        img[i] = (img[i] > threshold) ? 1 : 0;
    }
}

uint32_t mnist_image_info(const char* path, int* out_rows, int* out_cols) {
    FILE* f = fopen(path, "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file %s", path);
        return 0;
    }

    uint8_t header[16];
    if (fread(header, 1, 16, f) != 16) { 
        fclose(f); 
        ESP_LOGE(TAG, "Failed to read header from file %s", path);
        return 0;
    }

    uint32_t magic      = read_u32_be(&header[0]);
    uint32_t num_images = read_u32_be(&header[4]);
    uint32_t rows       = read_u32_be(&header[8]);
    uint32_t cols       = read_u32_be(&header[12]);

    if (magic != 0x00000803) { 
        fclose(f); 
        ESP_LOGE(TAG, "Invalid magic number in file %s", path);
        return 0;
    }

    *out_rows = (int)rows;
    *out_cols = (int)cols;

    fclose(f);

    return num_images;
}

uint8_t* mnist_load_image(FILE* f, int idx, int rows, int cols) {
    uint8_t header[16];
    fseek(f, 0, SEEK_SET);

    if (fread(header, 1, 16, f) != 16) { 
        fclose(f); 
        ESP_LOGE(TAG, "Failed to read header from file");
        return NULL; 
    }

    uint32_t magic      = read_u32_be(&header[0]);

    if (magic != 0x00000803) { 
        fclose(f); 
        ESP_LOGE(TAG, "Invalid magic number in file");
        return NULL; 
    }

    size_t total = (size_t)rows * cols;
    uint8_t* buf = (uint8_t*)malloc(total);
    if (!buf) { fclose(f); return NULL; }

    fseek(f, 16 + (size_t)idx * total, SEEK_SET);
    if (fread(buf, 1, total, f) != total) { free(buf); fclose(f); return NULL; }

    return buf;
}

uint32_t mnist_label_info(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file %s", path);
        return 0;
    }

    uint8_t header[8];
    if (fread(header, 1, 8, f) != 8) { 
        fclose(f); 
        ESP_LOGE(TAG, "Failed to read header from file %s", path);
        return 0; 
    }

    uint32_t magic      = read_u32_be(&header[0]);
    uint32_t num_labels = read_u32_be(&header[4]);

    if (magic != 0x00000801) { 
        fclose(f); 
        ESP_LOGE(TAG, "Invalid magic number in file %s", path);
        return 0; 
    }

    fclose(f);

    return num_labels;
}

int8_t mnist_load_label(FILE* f, int idx) {
    uint8_t header[8];
    fseek(f, 0, SEEK_SET);

    if (fread(header, 1, 8, f) != 8) { 
        fclose(f); 
        ESP_LOGE(TAG, "Failed to read header from file");
        return -1; 
    }

    uint32_t magic      = read_u32_be(&header[0]);

    if (magic != 0x00000801) { 
        fclose(f); 
        ESP_LOGE(TAG, "Invalid magic number in file");
        return -1; 
    }

    fseek(f, 8 + (size_t)idx, SEEK_SET);
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
