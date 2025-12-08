#include <sys/unistd.h>

#include <esp_log.h>
#include <esp_random.h>

#include "sdcard.h"
#include "mnist.h"

static const char *TAG = "main";

void app_main(void)
{
    // Initialize SD card and mount FAT filesystem
    sdmmc_card_t *card = sdcard_init();

    // list_recursive(MOUNT_POINT);

    // Use POSIX and C standard library functions to work with files.
    int rows, cols;
    uint32_t train_img_count = mnist_image_info(MOUNT_POINT"/train-images-idx3-ubyte", &rows, &cols);
    ESP_LOGI(TAG, "MNIST training set: %d images of size %dx%d", train_img_count, rows, cols);

    uint32_t train_label_count = mnist_label_info(MOUNT_POINT"/train-labels-idx1-ubyte");
    ESP_LOGI(TAG, "MNIST training set: %d labels", train_label_count);
    if (train_img_count != train_label_count) {
        ESP_LOGE(TAG, "Image count and label count do not match!");
    }

    uint32_t test_img_count = mnist_image_info(MOUNT_POINT"/t10k-images-idx3-ubyte", &rows, &cols);
    ESP_LOGI(TAG, "MNIST test set: %d images of size %dx%d", test_img_count, rows, cols);

    uint32_t test_label_count = mnist_label_info(MOUNT_POINT"/t10k-labels-idx1-ubyte");
    ESP_LOGI(TAG, "MNIST test set: %d labels", test_label_count);
    if (test_img_count != test_label_count) {
        ESP_LOGE(TAG, "Image count and label count do not match!");
    }

    // Get a random image index to load and print
    int img_index = esp_random() % train_img_count;
    ESP_LOGI(TAG, "Loading and printing training image %d", img_index);

    // Print mnist train image
    uint8_t* train_img = mnist_load_image(MOUNT_POINT"/train-images-idx3-ubyte", img_index, rows, cols);
    if (train_img) {
        mnist_print_img(train_img);
        int8_t train_label = mnist_load_label(MOUNT_POINT"/train-labels-idx1-ubyte", img_index);
        ESP_LOGI(TAG, "Training image label: %d", train_label);
        free(train_img);
    } 

    // Print mnist test image
    img_index = esp_random() % test_img_count;
    ESP_LOGI(TAG, "Loading and printing testing image %d", img_index);
    uint8_t* test_img = mnist_load_image(MOUNT_POINT"/t10k-images-idx3-ubyte", img_index, rows, cols);
    if (test_img) {
        mnist_print_img(test_img);
        int8_t test_label = mnist_load_label(MOUNT_POINT"/t10k-labels-idx1-ubyte", img_index);
        ESP_LOGI(TAG, "Testing image label: %d", test_label);
        free(test_img);
    }

    sdcard_deinit(card);
}
