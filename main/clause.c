#include "clause.h"

#include <esp_vfs_fat.h>

#include "tsetlin.pb-c.h"

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
