#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <mnist.h>
#include <tsetlin.h>

#include "sdcard.h"

LOG_MODULE_REGISTER(main);
static const char *TAG = "main";

void print_progress(const char *label, int percent) {
    const int bar_width = 40;
    int filled = percent * bar_width / 100;

    printk("%s [", label);
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) printk("=");
        else printk(" ");
    }
    printk("] %3d%%\r", percent);  // stay on same line
    fflush(stdout);
}

int main(void)
{
    int res = sdcard_init();
    
    if (res != FS_RET_OK) {
        printk("Error mounting disk.\n");
        return -1;
    }

    LOGI(TAG, "Disk mounted.\n");

    k_sleep(K_MSEC(5000));

    // Get training set info
    int rows, cols;
    uint32_t train_img_count = mnist_image_info(DISK_MOUNT_PT"/train-images-idx3-ubyte", &rows, &cols);
    LOGI(TAG, "MNIST training set: %d images of size %dx%d", train_img_count, rows, cols);

    uint32_t train_label_count = mnist_label_info(DISK_MOUNT_PT"/train-labels-idx1-ubyte");
    LOGI(TAG, "MNIST training set: %d labels", train_label_count);
    if (train_img_count != train_label_count) {
        LOGE(TAG, "Image count and label count do not match!");
        return -1;
    }

    // Get test set info
    uint32_t test_img_count = mnist_image_info(DISK_MOUNT_PT"/t10k-images-idx3-ubyte", &rows, &cols);
    LOGI(TAG, "MNIST test set: %d images of size %dx%d", test_img_count, rows, cols);

    uint32_t test_label_count = mnist_label_info(DISK_MOUNT_PT"/t10k-labels-idx1-ubyte");
    LOGI(TAG, "MNIST test set: %d labels", test_label_count);
    if (test_img_count != test_label_count) {
        LOGE(TAG, "Image count and label count do not match!");
        return -1;
    }

    if (train_img_count == 0 || test_img_count == 0) {
        LOGE(TAG, "No images found in dataset!");
        return -1;
    }

    // Print mnist train image
    int img_index = fast_rand() % train_img_count;
    printf("Loading and printing training image %d\n", img_index);

    FILE* f_train_imgs = fopen(DISK_MOUNT_PT"/train-images-idx3-ubyte", "rb");
    if (!f_train_imgs) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/train-images-idx3-ubyte");
        return -1;
    }

    FILE *f_train_labels = fopen(DISK_MOUNT_PT"/train-labels-idx1-ubyte", "rb");
    if (!f_train_labels) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/train-labels-idx1-ubyte");
        return -1;
    }

    uint8_t* train_img = mnist_load_image(f_train_imgs, img_index, rows, cols);
    if (train_img) {
        mnist_print_img(train_img);
        int8_t train_label = mnist_load_label(f_train_labels, img_index);
        printf("Training image label: %d\n", train_label);
        free(train_img);
    } 
    else {
        LOGE(TAG, "Failed to load train images");
        return -1;
    }

    // Print mnist test image
    img_index = fast_rand() % test_img_count;
    printf("Loading and printing testing image %d\n", img_index);

    FILE* f_test_imgs = fopen(DISK_MOUNT_PT"/t10k-images-idx3-ubyte", "rb");
    if (!f_test_imgs) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/t10k-images-idx3-ubyte");
        return -1;
    }

    FILE* f_test_labels = fopen(DISK_MOUNT_PT"/t10k-labels-idx1-ubyte", "rb");
    if (!f_test_labels) {
        LOGE(TAG, "Failed to open file %s", DISK_MOUNT_PT"/t10k-labels-idx1-ubyte");
        return -1;
    }

    uint8_t* test_img = mnist_load_image(f_test_imgs, img_index, rows, cols);
    if (test_img) {
        mnist_print_img(test_img);
        int8_t test_label = mnist_load_label(f_test_labels, img_index);
        printf("Testing image label: %d\n", test_label);
        free(test_img);
    }
    else {
        LOGE(TAG, "Failed to load test images");
        return -1;
    }

    // Load Tsetlin model from file
    size_t size = 0;
    uint8_t* data = tsetlin_read_file(DISK_MOUNT_PT"/tsetlin_model.cpb", &size);
    if (!data) {
        LOGE(TAG, "Failed to read file");
        return -1;
    }

    LOGI(TAG, "Model loaded (%d Bytes)", size);

    Tsetlin* model = tsetlin__unpack(NULL, size, data);
    free(data);

    if (!model) {
        LOGE(TAG, "Failed to unpack protobuf");
        return -1;
    }

    LOGI(TAG, "n_class   = %u", model->n_class);
    LOGI(TAG, "n_feature = %u", model->n_feature);
    LOGI(TAG, "n_clause  = %u", model->n_clause);
    LOGI(TAG, "n_state   = %u", model->n_state);
    LOGI(TAG, "model_type = %u", model->model_type);

    // Evaluate model on a random test image
    img_index = fast_rand() % test_img_count;

    uint8_t* img = mnist_load_image(f_test_imgs, img_index, rows, cols);
    if (!img) {
        LOGE(TAG, "Failed to load test image");
        tsetlin__free_unpacked(model, NULL);
        return -1;
    }

    int8_t label = mnist_load_label(f_test_labels, img_index);
    if (label < 0) {
        LOGE(TAG, "Failed to load test label");
        free(img);
        tsetlin__free_unpacked(model, NULL);
        return -1;
    }

    LOGI(TAG, "Evaluating model on test image %d (label %d)", img_index, label);
    mnist_print_img(img);

    int32_t votes[model->n_class];
    uint8_t predicted_class = 0;

    // Booleanize image using threshold 75
    // mnist_booleanize_img(img, rows * cols, 75);

    // Booleanize image using 8-bit representation
    uint8_t* bool_img = mnist_booleanize_img_n_bit(img, rows, cols, 8);
    free(img);

    // Evaluate
    tsetlin_evaluate(model, bool_img, votes, &predicted_class);

    LOGI(TAG, "Predicted class: %d with %d votes\n", predicted_class, votes[predicted_class]);
    for (size_t i = 0; i < model->n_class; i++)
    {
        LOGI(TAG, "Class %d: %d votes", i, votes[i]);
    }

    free(bool_img);

    // Evaluate on the entire test set
    int correct = 0;
    if (1) {

        long total_utility_time = 0;
        long total_calc_time = 0;

        for (uint32_t i = 0; i < test_img_count; i++)
        {
            uint32_t start_utility = k_uptime_get_32();

            if( i == 0) {
                // Skip the header
                fseek(f_test_imgs, 16, SEEK_SET);
                fseek(f_test_labels, 8, SEEK_SET);
            }

            uint8_t* img = mnist_load_next_image(f_test_imgs, i, rows, cols);
            if (!img) {
                LOGE(TAG, "Failed to load test image %d", i);
                continue;
            }

            int8_t label = mnist_load_next_label(f_test_labels, i);
            if (label < 0) {
                LOGE(TAG, "Failed to load test label %d", i);
                free(img);
                continue;
            }

            total_utility_time += (k_uptime_get_32() - start_utility);

            uint32_t start = k_uptime_get_32();

            // Booleanize image using threshold 75
            // mnist_booleanize_img(img, rows * cols, 75);

            // Booleanize image using 8-bit representation
            uint8_t* bool_img = mnist_booleanize_img_n_bit(img, rows, cols, 8);
            free(img);

            tsetlin_evaluate(model, bool_img, votes, &predicted_class);
            if (predicted_class == label) {
                correct++;
            }

            total_calc_time += (k_uptime_get_32() - start);

            free(bool_img);
            
            // Print progress every 1000 images
            if ((i + 1) % 1000 == 0) {
                char message[32];
                snprintf(message, sizeof(message), "Testing %d/%d", i + 1, test_img_count);
                print_progress(message, (i + 1) * 100 / test_img_count);
                // printf("Processed %d/%d test images\n", i + 1, test_img_count);
            }
        }
        LOGI(TAG, "");

        float tks = test_img_count / (double)(total_calc_time) * 1000;
        LOGI(TAG, "[TM] Achieved images/s: %d", k_ticks_to_ms_floor32(tks));

        float uts = test_img_count / (double)(total_utility_time) * 1000;
        LOGI(TAG, "[UM] Achieved images/s: %d", k_ticks_to_ms_floor32(uts));

        LOGI(TAG, "Accuracy on test set (%d): %.2f%%\n", test_img_count, (double)correct / test_img_count * 100);
    }

    // Test the random number generator speed
    const uint32_t N_RAND = 10;
    for (uint32_t i = 0; i < N_RAND; i++)
    {
        float r = random_float_01();
        LOGI(TAG, "Random float %d: %f", i, (double) r);
    }

    // Train the model on the training set
    uint32_t N_EPOCHS = 10;
    uint32_t T = 10;
    float s = 7.5f;
    
    for (size_t i = 0; i < N_EPOCHS; i++)
    {
        for (uint32_t j = 0; j < train_img_count; j++)
        {
            if( j == 0) {
                fseek(f_train_imgs, 16, SEEK_SET);
                fseek(f_train_labels, 8, SEEK_SET);
            }

            uint8_t* X_img = mnist_load_next_image(f_train_imgs, j, rows, cols);
            if (!X_img) {
                printf("Failed to load train image %d\n", j);
                continue;
            }

            int8_t y_target = mnist_load_next_label(f_train_labels, j);
            if (y_target < 0) {
                printf("Failed to load train label %d\n", j);
                continue;
            }

            // Booleanize image using threshold 75
            // mnist_booleanize_img(X_img, rows * cols, 75);

            // Booleanize image using 8-bit representation
            uint8_t* bool_img = mnist_booleanize_img_n_bit(X_img, rows, cols, 8);
            free(X_img);

            tsetlin_step(model, bool_img, y_target, T, s);
            free(bool_img);

            // Print progress every 1000 images
            if ((j + 1) % 1000 == 0) {
                char message[32];
                snprintf(message, sizeof(message), "Epoch %d: Processed %d/%d", i + 1, j + 1, train_img_count);
                print_progress(message, (j + 1) * 100 / train_img_count);
                // printf("Epoch %d: Processed %d/%d training images\n", i + 1, j + 1, train_img_count);
            }
        }

        LOGI(TAG, "\n");

        // Evaluate on test set after each epoch
        correct = 0;
        for (uint32_t j = 0; j < test_img_count; j++)
        {
            if( j == 0) {
                // Skip the header
                fseek(f_test_imgs, 16, SEEK_SET);
                fseek(f_test_labels, 8, SEEK_SET);
            }

            uint8_t* img = mnist_load_next_image(f_test_imgs, j, rows, cols);
            if (!img) {
                printf("Failed to load test image %d\n", j);
                continue;
            }

            int8_t label = mnist_load_next_label(f_test_labels, j);
            if (label < 0) {
                printf("Failed to load test label %d\n", j);
                free(img);
                continue;
            }

            // Booleanize image using threshold 75
            // mnist_booleanize_img(img, rows * cols, 75);

            // Booleanize image using 8-bit representation
            uint8_t* bool_img = mnist_booleanize_img_n_bit(img, rows, cols, 8);
            free(img);

            tsetlin_evaluate(model, bool_img, votes, &predicted_class);
            if (predicted_class == label) {
                correct++;
            }

            free(bool_img);

            // Print progress every 1000 images
            if ((j + 1) % 1000 == 0) {
                char message[32];
                snprintf(message, sizeof(message), "Testing %d/%d", j + 1, test_img_count);
                print_progress(message, (j + 1) * 100 / test_img_count);
                // printf("Processed %d/%d test images\n", j + 1, test_img_count);
            }
        }
        printf("\n");
        printf("Testing Accuracy after epoch %d: %.2f%%\n", i + 1, (double)correct / test_img_count * 100);
    }

    // free protobuf
    tsetlin__free_unpacked(model, NULL);

    fclose(f_train_imgs);
    fclose(f_test_imgs);

    fclose(f_train_labels);
    fclose(f_test_labels);

    sdcard_deinit();

    while (1) {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
