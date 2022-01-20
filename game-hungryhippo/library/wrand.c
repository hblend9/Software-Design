#include "wrand.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/*** STRUCTURES ***/
struct wrand {
    size_t bin_count;
    double *bin_his;
};

/*** PRIVATE PROTOTYPES ***/
double *_wrand_normalize_weights(size_t bin_count, const double *weights);

/*** DEFINITIONS ***/

wrand_t *wrand_init(size_t bin_count, const double *weights) {
    assert(bin_count > 0);
    double *weights_normed = _wrand_normalize_weights(bin_count, weights);
    wrand_t *wrand = malloc(sizeof(wrand_t));
    wrand->bin_count = bin_count;
    wrand->bin_his = calloc(bin_count, sizeof(double));
    wrand->bin_his[0] = weights_normed[0];
    for (size_t bin_idx = 1; bin_idx < bin_count; bin_idx++) {
        wrand->bin_his[bin_idx]
            = wrand->bin_his[bin_idx - 1] + weights_normed[bin_idx];
    }
    free(weights_normed);
    return wrand;
}

void wrand_free(wrand_t *wrand) {
    free(wrand->bin_his);
    free(wrand);
}

size_t wrand_sample(wrand_t *wrand) {
    assert(RAND_MAX > 0);
    double x = (double)rand() / (double)RAND_MAX;
    assert(x >= 0 && x <= 1);
    for (size_t bin_idx = 0; bin_idx < wrand->bin_count; bin_idx++) {
        if (x < wrand->bin_his[bin_idx]) {
            return bin_idx;
        }
    }
    return wrand->bin_count;
}

double *_wrand_normalize_weights(size_t bin_count, const double *weights) {
    double *weights_normed = calloc(bin_count, sizeof(double));
    double sum = 0;
    for (size_t i = 0; i < bin_count; i++) {
        sum += weights[i];
    }
    assert(sum != 0);
    for (size_t i = 0; i < bin_count; i++) {
        weights_normed[i] = weights[i] / sum;
    }
    return weights_normed;
}
