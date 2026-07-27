#ifndef RUNNER_H
#define RUNNER_H

#include "workload.h"

int runner_execute(
    const workload_t *workload,
    const char *target,
    const char *result_path
);

#endif
