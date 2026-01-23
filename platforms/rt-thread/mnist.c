#include <stdio.h>
#include <rtthread.h>

#include <mnist.h>
#include <tsetlin.h>

#include <fast_rand.h>

#define DISK_MOUNT_PT "/sdcard"

static const char *TAG = "main";

void print_progress(const char *label, int percent) {
    const int bar_width = 40;
    int filled = percent * bar_width / 100;

    printf("%s [", label);
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) printf("=");
        else printf(" ");
    }
    printf("] %3d%%\r", percent);  // stay on same line
    fflush(stdout);
}

static void lime_tm_mnist(int argc, char* argv[]) {
    // Get training set info
    int rows, cols;
    uint32_t train_img_count = mnist_image_info(DISK_MOUNT_PT"/train-images-idx3-ubyte", &rows, &cols);
    LOGI(TAG, "MNIST training set: %d images of size %dx%d", train_img_count, rows, cols);

    uint32_t train_label_count = mnist_label_info(DISK_MOUNT_PT"/train-labels-idx1-ubyte");
    LOGI(TAG, "MNIST training set: %d labels", train_label_count);
    if (train_img_count != train_label_count) {
        LOGE(TAG, "Image count and label count do not match!");
        return;
    }

    // Get test set info
    uint32_t test_img_count = mnist_image_info(DISK_MOUNT_PT"/t10k-images-idx3-ubyte", &rows, &cols);
    LOGI(TAG, "MNIST test set: %d images of size %dx%d", test_img_count, rows, cols);

    uint32_t test_label_count = mnist_label_info(DISK_MOUNT_PT"/t10k-labels-idx1-ubyte");
    LOGI(TAG, "MNIST test set: %d labels", test_label_count);
    if (test_img_count != test_label_count) {
        LOGE(TAG, "Image count and label count do not match!");
        return;
    }

    if (train_img_count == 0 || test_img_count == 0) {
        LOGE(TAG, "No images found in dataset!");
        return;
    }

   // Print mnist train image
    int img_index = fast_rand() % train_img_count;
    LOGI(TAG, "Loading and printing training image %d", img_index);

    FILE* f_train_imgs = fopen(DISK_MOUNT_PT"/train-images-idx3-ubyte", "r");
    if (!f_train_imgs) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/train-images-idx3-ubyte");
        return;
    }

    FILE *f_train_labels = fopen(DISK_MOUNT_PT"/train-labels-idx1-ubyte", "r");
    if (!f_train_labels) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/train-labels-idx1-ubyte");
        return;
    }

    uint8_t* train_img = mnist_load_image(f_train_imgs, img_index, rows, cols);
    if (train_img) {
        mnist_print_img(train_img);
        int8_t train_label = mnist_load_label(f_train_labels, img_index);
        LOGI(TAG, "Training image label: %d", train_label);
        free(train_img);
    } 

    // Print mnist test image
    img_index = fast_rand() % test_img_count;
    LOGI(TAG, "Loading and printing testing image %d", img_index);

    FILE* f_test_imgs = fopen(DISK_MOUNT_PT"/t10k-images-idx3-ubyte", "r");
    if (!f_test_imgs) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/t10k-images-idx3-ubyte");
        return;
    }

    FILE* f_test_labels = fopen(DISK_MOUNT_PT"/t10k-labels-idx1-ubyte", "r");
    if (!f_test_labels) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/t10k-labels-idx1-ubyte");
        return;
    }

    uint8_t* test_img = mnist_load_image(f_test_imgs, img_index, rows, cols);
    if (test_img) {
        mnist_print_img(test_img);
        int8_t test_label = mnist_load_label(f_test_labels, img_index);
        LOGI(TAG, "Testing image label: %d", test_label);
        free(test_img);
    }

	// Load Tsetlin model from file
    size_t size = 0;
    uint8_t* data = tsetlin_read_file(DISK_MOUNT_PT"/tsetlin_model.cpb", &size);
    if (!data) {
        LOGE(TAG, "Failed to read file\n");
        return;
    }

    Tsetlin* model = tsetlin__unpack(NULL, size, data);
    free(data);

    if (!model) {
        LOGE(TAG, "Failed to unpack protobuf\n");
        return;
    }

    printf("n_class   = %u\n", model->n_class);
    printf("n_feature = %u\n", model->n_feature);
    printf("n_clause  = %u\n", model->n_clause);
    printf("n_state   = %u\n", model->n_state);
    printf("model_type = %u\n", model->model_type);

	return;
}
MSH_CMD_EXPORT(lime_tm_mnist, LiME-TM mnist training and testing example);