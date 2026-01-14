#include <sys/unistd.h>

#include <esp_log.h>
#include <esp_random.h>

#include "sdcard.h"
#include "mnist.h"

#include "tsetlin.h"
#include "clause.h"
#include "tsetlin.pb-c.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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

void app_main(void)
{
    // Initialize SD card and mount FAT filesystem
    sdmmc_card_t *card = sdcard_init();

    // Get training set info
    int rows, cols;
    uint32_t train_img_count = mnist_image_info(MOUNT_POINT"/train-images-idx3-ubyte", &rows, &cols);
    ESP_LOGI(TAG, "MNIST training set: %d images of size %dx%d", train_img_count, rows, cols);

    uint32_t train_label_count = mnist_label_info(MOUNT_POINT"/train-labels-idx1-ubyte");
    ESP_LOGI(TAG, "MNIST training set: %d labels", train_label_count);
    if (train_img_count != train_label_count) {
        ESP_LOGE(TAG, "Image count and label count do not match!");
        return;
    }

    // Get test set info
    uint32_t test_img_count = mnist_image_info(MOUNT_POINT"/t10k-images-idx3-ubyte", &rows, &cols);
    ESP_LOGI(TAG, "MNIST test set: %d images of size %dx%d", test_img_count, rows, cols);

    uint32_t test_label_count = mnist_label_info(MOUNT_POINT"/t10k-labels-idx1-ubyte");
    ESP_LOGI(TAG, "MNIST test set: %d labels", test_label_count);
    if (test_img_count != test_label_count) {
        ESP_LOGE(TAG, "Image count and label count do not match!");
        return;
    }

    if (train_img_count == 0 || test_img_count == 0) {
        ESP_LOGE(TAG, "No images found in dataset!");
        return;
    }

    // Print mnist train image
    int img_index = esp_random() % train_img_count;
    ESP_LOGI(TAG, "Loading and printing training image %d", img_index);

    FILE* f_train_imgs = fopen(MOUNT_POINT"/train-images-idx3-ubyte", "r");
    if (!f_train_imgs) {
        ESP_LOGE(TAG, "Failed to open file %s", MOUNT_POINT"/train-images-idx3-ubyte");
        return;
    }

    FILE *f_train_labels = fopen(MOUNT_POINT"/train-labels-idx1-ubyte", "r");
    if (!f_train_labels) {
        ESP_LOGE(TAG, "Failed to open file %s", MOUNT_POINT"/train-labels-idx1-ubyte");
        return;
    }

    uint8_t* train_img = mnist_load_image(f_train_imgs, img_index, rows, cols);
    if (train_img) {
        mnist_print_img(train_img);
        int8_t train_label = mnist_load_label(f_train_labels, img_index);
        ESP_LOGI(TAG, "Training image label: %d", train_label);
        free(train_img);
    } 

    // Print mnist test image
    img_index = esp_random() % test_img_count;
    ESP_LOGI(TAG, "Loading and printing testing image %d", img_index);

    FILE* f_test_imgs = fopen(MOUNT_POINT"/t10k-images-idx3-ubyte", "r");
    if (!f_test_imgs) {
        ESP_LOGE(TAG, "Failed to open file %s", MOUNT_POINT"/t10k-images-idx3-ubyte");
        return;
    }

    FILE* f_test_labels = fopen(MOUNT_POINT"/t10k-labels-idx1-ubyte", "r");
    if (!f_test_labels) {
        ESP_LOGE(TAG, "Failed to open file %s", MOUNT_POINT"/t10k-labels-idx1-ubyte");
        return;
    }

    uint8_t* test_img = mnist_load_image(f_test_imgs, img_index, rows, cols);
    if (test_img) {
        mnist_print_img(test_img);
        int8_t test_label = mnist_load_label(f_test_labels, img_index);
        ESP_LOGI(TAG, "Testing image label: %d", test_label);
        free(test_img);
    }

    // Load Tsetlin model from file
    size_t size = 0;
    uint8_t* data = tsetlin_read_file(MOUNT_POINT"/tsetlin_model_8_bit.cpb", &size);
    if (!data) {
        printf("Failed to read file\n");
        return;
    }

    Tsetlin* model = tsetlin__unpack(NULL, size, data);
    free(data);

    if (!model) {
        printf("Failed to unpack protobuf\n");
        return;
    }

    printf("n_class   = %lu\n", model->n_class);
    printf("n_feature = %lu\n", model->n_feature);
    printf("n_clause  = %lu\n", model->n_clause);
    printf("n_state   = %lu\n", model->n_state);
    printf("model_type = %u\n", model->model_type);

    // Evaluate model on a random test image
    img_index = esp_random() % test_img_count;

    uint8_t* img = mnist_load_image(f_test_imgs, img_index, rows, cols);
    if (!img) {
        printf("Failed to load test image\n");
        tsetlin__free_unpacked(model, NULL);
        return;
    }

    int8_t label = mnist_load_label(f_test_labels, img_index);
    if (label < 0) {
        printf("Failed to load test label\n");
        free(img);
        tsetlin__free_unpacked(model, NULL);
        return;
    }

    printf("Evaluating model on test image %d (label %d)\n", img_index, label);
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

    printf("Predicted class: %d with %ld votes\n", predicted_class, votes[predicted_class]);
    for (size_t i = 0; i < model->n_class; i++)
    {
        printf("Class %d: %ld votes\n", i, votes[i]);
    }

    free(bool_img);

    // Evaluate on the entire test set
    int correct = 0;
    if (1) {

        long total_utility_time = 0;
        long total_calc_time = 0;

        for (uint32_t i = 0; i < test_img_count; i++)
        {
            TickType_t start_utility = xTaskGetTickCount();

            if( i == 0) {
                // Skip the header
                fseek(f_test_imgs, 16, SEEK_SET);
                fseek(f_test_labels, 8, SEEK_SET);
            }

            uint8_t* img = mnist_load_next_image(f_test_imgs, i, rows, cols);
            if (!img) {
                printf("Failed to load test image %ld\n", i);
                continue;
            }

            int8_t label = mnist_load_next_label(f_test_labels, i);
            if (label < 0) {
                printf("Failed to load test label %ld\n", i);
                free(img);
                continue;
            }

            total_utility_time += (xTaskGetTickCount() - start_utility);

            TickType_t start = xTaskGetTickCount();

            // Booleanize image using threshold 75
            // mnist_booleanize_img(img, rows * cols, 75);

            // Booleanize image using 8-bit representation
            uint8_t* bool_img = mnist_booleanize_img_n_bit(img, rows, cols, 8);
            free(img);

            tsetlin_evaluate(model, bool_img, votes, &predicted_class);
            if (predicted_class == label) {
                correct++;
            }

            total_calc_time += (xTaskGetTickCount() - start);

            free(bool_img);
            
            // Print progress every 1000 images
            if ((i + 1) % 1000 == 0) {
                char message[32];
                snprintf(message, sizeof(message), "Testing %ld/%ld", i + 1, test_img_count);
                print_progress(message, (i + 1) * 100 / test_img_count);
                // printf("Processed %ld/%ld test images\n", i + 1, test_img_count);
            }
        }
        printf("\n");

        float tks = test_img_count / (double)(total_calc_time) * 1000;
        printf("[TM] Achieved images/s: %f\n", tks / portTICK_PERIOD_MS);

        float uts = test_img_count / (double)(total_utility_time) * 1000;
        printf("[UM] Achieved images/s: %f\n", uts / portTICK_PERIOD_MS);

        printf("Accuracy on test set (%ld): %.2f%%\n", test_img_count, (double)correct / test_img_count * 100);
    }

    // Test the random number generator speed
    const uint32_t N_RAND = 10;
    for (uint32_t i = 0; i < N_RAND; i++)
    {
        float r = random_float_01();
        printf("Random float %ld: %f\n", i, r);
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
                printf("Failed to load train image %ld\n", j);
                continue;
            }

            int8_t y_target = mnist_load_next_label(f_train_labels, j);
            if (y_target < 0) {
                printf("Failed to load train label %ld\n", j);
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
                snprintf(message, sizeof(message), "Epoch %d: Processed %ld/%ld", i + 1, j + 1, train_img_count);
                print_progress(message, (j + 1) * 100 / train_img_count);
                // printf("Epoch %d: Processed %ld/%ld training images\n", i + 1, j + 1, train_img_count);
            }
        }

        printf("\n");

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
                printf("Failed to load test image %ld\n", j);
                continue;
            }

            int8_t label = mnist_load_next_label(f_test_labels, j);
            if (label < 0) {
                printf("Failed to load test label %ld\n", j);
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
                snprintf(message, sizeof(message), "Testing %ld/%ld", j + 1, test_img_count);
                print_progress(message, (j + 1) * 100 / test_img_count);
                // printf("Processed %ld/%ld test images\n", j + 1, test_img_count);
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

    sdcard_deinit(card);
}
