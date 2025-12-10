#include <sys/unistd.h>

#include <esp_log.h>
#include <esp_random.h>

#include "sdcard.h"
#include "mnist.h"

#include "tsetlin.pb-c.h"


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "main";

uint8_t* read_file(const char* path, size_t* out_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* buffer = malloc(size);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    fread(buffer, 1, size, f);
    fclose(f);

    *out_size = size;
    return buffer;
}

uint8_t clause_evaluate(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature) {
    for (size_t k = 0; k < clause->n_pos_literal; k++)
    {
        uint32_t idx_literal = clause->position[k];
        if (clause->data[k] > n_state / 2)
        {
            // positive literal is included
            if (input[idx_literal] <= 75)
            {
                return 0; // Clause evaluates to false
            }
        }
    }

    for (size_t k = 0; k < clause->n_neg_literal; k++)
    {
        uint32_t idx_literal = clause->position[clause->n_pos_literal + k];
        if (clause->data[clause->n_pos_literal + k] > n_state / 2)
        {
            // negative literal is included
            if (input[idx_literal] > 75)
            {
                return 0; // Clause evaluates to false
            }
        }
    }

    return 1; // Clause evaluates to true
}

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

    size_t size = 0;
    uint8_t* data = read_file(MOUNT_POINT"/tsetlin_model.cpb", &size);
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
    memset(votes, 0, sizeof(votes));

    for (size_t i = 0; i < model->n_class; i++)
    {
        for (size_t j = 0; j <(size_t) model->n_clause / 2; j++)
        {
            ClauseCompressed* p_clause = model->clauses_compressed[i * model->n_clause + j * 2];
            ClauseCompressed* n_clause = model->clauses_compressed[i * model->n_clause + j * 2 + 1];
            
            votes[i] += clause_evaluate(p_clause, img, model->n_state, model->n_feature);
            votes[i] -= clause_evaluate(n_clause, img, model->n_state, model->n_feature);
        }
    }

    // Print votes for each class
    for (size_t i = 0; i < model->n_class; i++)
    {
        printf("Class %u: %ld votes\n", i, votes[i]);
    }

    // Find predicted class
    uint8_t predicted_class = 0;
    int32_t max_votes = votes[0];
    for (size_t i = 1; i < model->n_class; i++)
    {
        if (votes[i] > max_votes)
        {
            max_votes = votes[i];
            predicted_class = i;
        }
    }
    
    printf("Predicted class: %d with %ld votes\n", predicted_class, max_votes);
    free(img);

    // Evaluate on the entire test set
    int correct = 0;
    uint32_t num_test = 10000;

    long total_utility_time = 0;
    long total_calc_time = 0;

    for (uint32_t i = 0; i < num_test; i++)
    {
        TickType_t start_utility = xTaskGetTickCount();
        uint8_t* img = mnist_load_image(f_test_imgs, i, rows, cols);
        if (!img) {
            printf("Failed to load test image %ld\n", i);
            continue;
        }
        int8_t label = mnist_load_label(f_test_labels, i);
        if (label < 0) {
            printf("Failed to load test label %ld\n", i);
            free(img);
            continue;
        }

        memset(votes, 0, sizeof(votes));
        total_utility_time += (xTaskGetTickCount() - start_utility);

        TickType_t start = xTaskGetTickCount();
        for (size_t c = 0; c < model->n_class; c++)
        {
            for (size_t j = 0; j <(size_t) model->n_clause / 2; j++)
            {
                ClauseCompressed* p_clause = model->clauses_compressed[c * model->n_clause + j * 2];
                ClauseCompressed* n_clause = model->clauses_compressed[c * model->n_clause + j * 2 + 1];
                
                votes[c] += clause_evaluate(p_clause, img, model->n_state, model->n_feature);
                votes[c] -= clause_evaluate(n_clause, img, model->n_state, model->n_feature);
            }
        }

        // Find predicted class
        uint8_t predicted_class = 0;
        int32_t max_votes = votes[0];
        for (size_t c = 1; c < model->n_class; c++)
        {
            if (votes[c] > max_votes)
            {
                max_votes = votes[c];
                predicted_class = c;
            }
        }

        if (predicted_class == label) {
            correct++;
        }

        total_calc_time += (xTaskGetTickCount() - start);

        free(img);
        
        // Print progress every 100 images
        if ((i + 1) % 100 == 0) {
            printf("Processed %ld/%ld test images\n", i + 1, num_test);
        }
    }

    float tks = num_test / (double)(total_calc_time) * 1000;
    printf("[TM] Achieved images/s: %f\n", tks / portTICK_PERIOD_MS);

    float uts = num_test / (double)(total_utility_time) * 1000;
    printf("[UM] Achieved images/s: %f\n", uts / portTICK_PERIOD_MS);

    printf("Accuracy on test set (%ld): %.2f%%\n", num_test, (double)correct / num_test * 100);

    // free protobuf
    tsetlin__free_unpacked(model, NULL);

    fclose(f_train_imgs);
    fclose(f_test_imgs);

    fclose(f_train_labels);
    fclose(f_test_labels);

    sdcard_deinit(card);
}
