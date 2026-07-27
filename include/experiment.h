#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "result.h"
#include "workload.h"

#include <stddef.h>

typedef struct {
    unsigned queue_depth;
    benchmark_result_t result;
} experiment_result_t;

typedef struct {
    experiment_result_t *items;
    size_t count;
} experiment_series_t;

int experiment_run_queue_depth_sweep(
    const workload_t *base_workload,
    const char *target_path,
    const char *results_directory,
    experiment_series_t *series
);

void experiment_print_summary(
    const experiment_series_t *series
);

int experiment_write_csv(
    const char *csv_path,
    const workload_t *base_workload,
    const experiment_series_t *series
);

void experiment_series_free(
    experiment_series_t *series
);

#endif
