#include "tsetlin.h"

#if defined(__ZEPHYR__)
    LOG_MODULE_REGISTER(tsetlin);
#endif

static const char* TAG = "tsetlin";

uint8_t* tsetlin_read_file(const char* path, size_t* out_size) {
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

void tsetlin_step(Tsetlin* model, uint8_t* X_img, int8_t y_target, uint32_t T, float s) {
    // Pair 1: Target class
    int32_t class_sum = 0;
    
    //int8_t pos_clauses_eval[model->n_clause / 2];
    int8_t* pos_clauses_eval = (int8_t*)malloc(sizeof(int8_t) * model->n_clause / 2);
    if (!pos_clauses_eval) {
       LOGE(TAG, "Failed to allocate memory for pos clauses!");
       return;
    }
    memset(pos_clauses_eval, 0, sizeof(int8_t) * model->n_clause / 2);

    //int8_t neg_clauses_eval[model->n_clause / 2];
    int8_t* neg_clauses_eval = (int8_t*)malloc(sizeof(int8_t) * model->n_clause / 2);
    if (!neg_clauses_eval) {
        LOGE(TAG, "Failed to allocate memory for neg clauses!");
        return;
    }
    memset(neg_clauses_eval, 0, sizeof(int8_t) * model->n_clause / 2);

    for (size_t i = 0; i <(size_t) model->n_clause / 2; i++)
    {
        ClauseCompressed* p_clause = model->clauses_compressed[y_target * model->n_clause + i * 2];
        ClauseCompressed* n_clause = model->clauses_compressed[y_target * model->n_clause + i * 2 + 1];

        pos_clauses_eval[i] = clause_evaluate(p_clause, X_img, model->n_state, model->n_feature);
        neg_clauses_eval[i] = clause_evaluate(n_clause, X_img, model->n_state, model->n_feature);

        class_sum += pos_clauses_eval[i];
        class_sum -= neg_clauses_eval[i];
    }

    // Clamp class_sum to [-T, T]
    if (class_sum > (int32_t)T) {
        class_sum = T;
    } else if (class_sum < -(int32_t)T) {
        class_sum = -T;
    }

    // Calculate probabilities
    float c1 = (T - class_sum) / (2 * T);

    // Update clauses for the target class
    for (size_t i = 0; i <(size_t) model->n_clause / 2; i++) {
        ClauseCompressed* p_clause = model->clauses_compressed[y_target * model->n_clause + i * 2];
        ClauseCompressed* n_clause = model->clauses_compressed[y_target * model->n_clause + i * 2 + 1];

        // Positive Clause: Type I Feedback
        if (random_float_01() <= c1)
            clause_update_type_I(p_clause, X_img, pos_clauses_eval[i], model->n_state, model->n_feature, s);

        // Negative Clause: Type II Feedback
        if (neg_clauses_eval[i] == 1 && (random_float_01() <= c1))
            clause_update_type_II(n_clause, X_img, model->n_state, model->n_feature);
    }

    // Pair 2: Non-target classes
    uint8_t other_class = y_target;
    while (other_class == y_target) {
        #if defined(ESP_PLATFORM)
            /* ESP-IDF */
            other_class = esp_random() % model->n_class;
        #elif defined(__ZEPHYR__)
            /* Zephyr RTOS */
            other_class = fast_rand() % model->n_class;
        #elif defined(__RTTHREAD__)
            /* RT-Thread RTOS */
            other_class = fast_rand() % model->n_class;
        #else
            other_class = fast_rand() % model->n_class;
        #endif
    }

    class_sum = 0;
    memset(pos_clauses_eval, 0, sizeof(int8_t) * model->n_clause / 2);
    memset(neg_clauses_eval, 0, sizeof(int8_t) * model->n_clause / 2);
    for (size_t i = 0; i <(size_t) model->n_clause / 2; i++)
    {
        ClauseCompressed* p_clause = model->clauses_compressed[other_class * model->n_clause + i * 2];
        ClauseCompressed* n_clause = model->clauses_compressed[other_class * model->n_clause + i * 2 + 1];

        pos_clauses_eval[i] = clause_evaluate(p_clause, X_img, model->n_state, model->n_feature);
        neg_clauses_eval[i] = clause_evaluate(n_clause, X_img, model->n_state, model->n_feature);

        class_sum += pos_clauses_eval[i];
        class_sum -= neg_clauses_eval[i];
    }

    // Clamp class_sum to [-T, T]
    if (class_sum > (int32_t)T) {
        class_sum = T;
    } else if (class_sum < -(int32_t)T) {
        class_sum = -T;
    }

    float c2 = (T + class_sum) / (2 * T);
    for( size_t i = 0; i <(size_t) model->n_clause / 2; i++) {
        ClauseCompressed* p_clause = model->clauses_compressed[other_class * model->n_clause + i * 2];
        ClauseCompressed* n_clause = model->clauses_compressed[other_class * model->n_clause + i * 2 + 1];

        // Positive Clause: Type II Feedback
        if (pos_clauses_eval[i] == 1 && (random_float_01() <= c2)) {
            clause_update_type_II(p_clause, X_img, model->n_state, model->n_feature);
        }

        // Negative Clause: Type I Feedback
        if (neg_clauses_eval[i] == 1 && (random_float_01() <= c2)) {
            clause_update_type_I(n_clause, X_img, neg_clauses_eval[i], model->n_state, model->n_feature, s);
        }
    }
}

int tsetlin_evaluate(Tsetlin* model, uint8_t* input, int32_t *out_votes, uint8_t* out_class) {
    memset(out_votes, 0, model->n_class * sizeof(int32_t));

    for (size_t c = 0; c < model->n_class; c++)
    {
        for (uint32_t j = 0; j <(size_t) model->n_clause / 2; j++)
        {
            ClauseCompressed* p_clause = model->clauses_compressed[c * model->n_clause + j * 2];
            ClauseCompressed* n_clause = model->clauses_compressed[c * model->n_clause + j * 2 + 1];

            out_votes[c] += clause_evaluate(p_clause, input, model->n_state, model->n_feature);
            out_votes[c] -= clause_evaluate(n_clause, input, model->n_state, model->n_feature);
        }
    }

    // Find class with maximum votes
    uint8_t max_class = 0;
    int32_t max_votes = out_votes[0];
    for (size_t c = 1; c < model->n_class; c++)
    {
        if (out_votes[c] > max_votes)
        {
            max_votes = out_votes[c];
            max_class = c;
        }
    }

    *out_class = max_class;

    return 0;
}
