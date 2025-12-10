#include "tsetlin.h"
#include "clause.h"

#include "tsetlin.pb-c.h"

#include <esp_vfs_fat.h>

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

int tsetlin_evaluate(Tsetlin* model, uint8_t* input, int32_t *out_votes, uint8_t* out_class) {
    memset(out_votes, 0, model->n_class * sizeof(int32_t));

    for (size_t c = 0; c < model->n_class; c++)
    {
        for (size_t j = 0; j <(size_t) model->n_clause / 2; j++)
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
