#ifndef POLICY_H
#define POLICY_H

#include "experiment.h"

typedef enum {
    POLICY_MAX_IOPS = 0,
    POLICY_MIN_LATENCY,
    POLICY_BALANCED,
    POLICY_INVALID
} policy_type_t;

typedef struct {
    policy_type_t policy;
    size_t selected_index;
    experiment_result_t selected_result;
    double score;
} policy_decision_t;

const char *policy_type_to_string(
    policy_type_t policy
);

policy_type_t policy_type_from_string(
    const char *value
);

int policy_select(
    policy_type_t policy,
    const experiment_series_t *series,
    policy_decision_t *decision
);

void policy_print_decision(
    const policy_decision_t *decision
);

#endif
