#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    WORKLOAD_RANDOM_READ = 0,
    WORKLOAD_RANDOM_WRITE,
    WORKLOAD_SEQUENTIAL_READ,
    WORKLOAD_SEQUENTIAL_WRITE,
    WORKLOAD_MIXED,
    WORKLOAD_INVALID
} workload_type_t;

typedef struct {
    workload_type_t type;
    const char *name;
    const char *fio_rw;
    unsigned block_size_bytes;
    unsigned queue_depth;
    unsigned jobs;
    unsigned read_percentage;
    unsigned runtime_seconds;
    bool random;
} workload_t;

const char *workload_type_to_string(workload_type_t type);
workload_type_t workload_type_from_string(const char *value);
int workload_create_default(workload_type_t type, workload_t *workload);
void workload_print(const workload_t *workload);
void workload_print_supported(void);

#endif
