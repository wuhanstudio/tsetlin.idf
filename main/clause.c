#include "clause.h"

#include <esp_vfs_fat.h>

#include "tsetlin.pb-c.h"

float random_float_01(void) {
    uint32_t r = esp_random();
    return (float)r / ((float)UINT32_MAX + 1.0);
}

void clause_update_type_I(ClauseCompressed* clause, uint8_t* input, int8_t clause_output, uint32_t n_state, uint32_t n_feature, float s) {
    // Want clause_output to be 1
    float s1 = 1 / s;
    float s2 = (s - 1) / s;

    // Erase Pattern
    // Reduce the number of included literals
    if (clause_output == 0) {
        // Update positive literals
        for (size_t k = 0; k < clause->n_pos_literal; k++)
        {
            uint32_t idx_literal = clause->position[k];
            if ( clause->data[k] > 1 && random_float_01() <= s1)
            {
                // Decrease state for included positive literal
                clause->data[k]--;
            }
        }

        // Update negative literals
        for (size_t k = 0; k < clause->n_neg_literal; k++)
        {
            uint32_t idx_literal = clause->position[clause->n_pos_literal + k];
            if (clause->data[clause->n_pos_literal + k] > 1 && random_float_01() <= s1)
            {
                // Decrease state for included negative literal
                clause->data[clause->n_pos_literal + k]--;
            }
        }
    }

    // Recognize Pattern
    // Increase the number of included literals
    if (clause_output == 1) {
        // Update positive literals
        for (size_t k = 0; k < clause->n_pos_literal; k++)
        {
            uint32_t idx_literal = clause->position[k];
            if (input[idx_literal] == 1 && clause->data[k] < n_state && random_float_01() <= s2)
            {
                // Increase state for included positive literal
                clause->data[k]++;
            }
            else if (input[idx_literal] == 0 && clause->data[k] > 1 && random_float_01() <= s1)
            {
                // Decrease state for excluded positive literal
                clause->data[k]--;
            }
        }

        // Update negative literals
        for (size_t k = 0; k < clause->n_neg_literal; k++)
        {
            uint32_t idx_literal = clause->position[clause->n_pos_literal + k];
            if (input[idx_literal] == 1 && clause->data[clause->n_pos_literal + k] > 1 && random_float_01() <= s1)
            {
                // Decrease state for included negative literal
                clause->data[clause->n_pos_literal + k]--;
            }
            else if (input[idx_literal] == 0 && clause->data[clause->n_pos_literal + k] < n_state && random_float_01() <= s2)
            {
                // Increase state for excluded negative literal
                clause->data[clause->n_pos_literal + k]++;
            }
        }
    }
}

void clause_update_type_II(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature) {
    // Update positive literals
    for (size_t k = 0; k < clause->n_pos_literal; k++)
    {
        uint32_t idx_literal = clause->position[k];
        if (input[idx_literal] == 0 && clause->data[k] <= n_state / 2)
        {
            // Increase state for included positive literal
            if (clause->data[k] < n_state) {
                clause->data[k]++;
            }
        }
    }

    // Update negative literals
    for (size_t k = 0; k < clause->n_neg_literal; k++)
    {
        uint32_t idx_literal = clause->position[clause->n_pos_literal + k];
        if (input[idx_literal] == 1 && clause->data[clause->n_pos_literal + k] <= n_state / 2)
        {
            // Increase state for included negative literal
            if (clause->data[clause->n_pos_literal + k] < n_state) {
                clause->data[clause->n_pos_literal + k]++;
            }
        }
    }
}

uint8_t clause_evaluate(ClauseCompressed* clause, uint8_t* input, uint32_t n_state, uint32_t n_feature) {
    for (size_t k = 0; k < clause->n_pos_literal; k++)
    {
        uint32_t idx_literal = clause->position[k];
        if (clause->data[k] > n_state / 2)
        {
            // positive literal is included
            if (input[idx_literal] == 0)
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
            if (input[idx_literal] == 1)
            {
                return 0; // Clause evaluates to false
            }
        }
    }

    return 1; // Clause evaluates to true
}
