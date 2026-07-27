#include "workload.h"

#include <stdio.h>
#include <string.h>

static int string_equals(const char *left, const char *right)
{
    return left != NULL && right != NULL && strcmp(left, right) == 0;
}

const char *workload_type_to_string(workload_type_t type)
{
    switch (type) {
    case WORKLOAD_RANDOM_READ:
        return "randread";
    case WORKLOAD_RANDOM_WRITE:
        return "randwrite";
    case WORKLOAD_SEQUENTIAL_READ:
        return "seqread";
    case WORKLOAD_SEQUENTIAL_WRITE:
        return "seqwrite";
    case WORKLOAD_MIXED:
        return "mixed";
    case WORKLOAD_INVALID:
    default:
        return "invalid";
    }
}

workload_type_t workload_type_from_string(const char *value)
{
    if (string_equals(value, "randread")) {
        return WORKLOAD_RANDOM_READ;
    }

    if (string_equals(value, "randwrite")) {
        return WORKLOAD_RANDOM_WRITE;
    }

    if (string_equals(value, "seqread") || string_equals(value, "read")) {
        return WORKLOAD_SEQUENTIAL_READ;
    }

    if (string_equals(value, "seqwrite") || string_equals(value, "write")) {
        return WORKLOAD_SEQUENTIAL_WRITE;
    }

    if (string_equals(value, "mixed") || string_equals(value, "randrw")) {
        return WORKLOAD_MIXED;
    }

    return WORKLOAD_INVALID;
}

int workload_create_default(workload_type_t type, workload_t *workload)
{
    if (workload == NULL) {
        return -1;
    }

    switch (type) {
    case WORKLOAD_RANDOM_READ:
        *workload = (workload_t) {
            .type = type,
            .name = "4 KiB random read",
            .fio_rw = "randread",
            .block_size_bytes = 4096U,
            .queue_depth = 8U,
            .jobs = 1U,
            .read_percentage = 100U,
            .runtime_seconds = 10U,
            .random = true
        };
        return 0;

    case WORKLOAD_RANDOM_WRITE:
        *workload = (workload_t) {
            .type = type,
            .name = "4 KiB random write",
            .fio_rw = "randwrite",
            .block_size_bytes = 4096U,
            .queue_depth = 8U,
            .jobs = 1U,
            .read_percentage = 0U,
            .runtime_seconds = 10U,
            .random = true
        };
        return 0;

    case WORKLOAD_SEQUENTIAL_READ:
        *workload = (workload_t) {
            .type = type,
            .name = "128 KiB sequential read",
            .fio_rw = "read",
            .block_size_bytes = 131072U,
            .queue_depth = 8U,
            .jobs = 1U,
            .read_percentage = 100U,
            .runtime_seconds = 10U,
            .random = false
        };
        return 0;

    case WORKLOAD_SEQUENTIAL_WRITE:
        *workload = (workload_t) {
            .type = type,
            .name = "128 KiB sequential write",
            .fio_rw = "write",
            .block_size_bytes = 131072U,
            .queue_depth = 8U,
            .jobs = 1U,
            .read_percentage = 0U,
            .runtime_seconds = 10U,
            .random = false
        };
        return 0;

    case WORKLOAD_MIXED:
        *workload = (workload_t) {
            .type = type,
            .name = "4 KiB 70/30 random mixed workload",
            .fio_rw = "randrw",
            .block_size_bytes = 4096U,
            .queue_depth = 16U,
            .jobs = 1U,
            .read_percentage = 70U,
            .runtime_seconds = 10U,
            .random = true
        };
        return 0;

    case WORKLOAD_INVALID:
    default:
        return -1;
    }
}

void workload_print(const workload_t *workload)
{
    if (workload == NULL) {
        return;
    }

    printf("Selected workload\n");
    printf("-----------------\n");
    printf("Type             : %s\n", workload_type_to_string(workload->type));
    printf("Description      : %s\n", workload->name);
    printf("fio rw mode      : %s\n", workload->fio_rw);
    printf("Block size       : %u bytes\n", workload->block_size_bytes);
    printf("Queue depth      : %u\n", workload->queue_depth);
    printf("Jobs             : %u\n", workload->jobs);
    printf("Read percentage  : %u%%\n", workload->read_percentage);
    printf("Runtime          : %u seconds\n", workload->runtime_seconds);
    printf("Access pattern   : %s\n", workload->random ? "random" : "sequential");
}

void workload_print_supported(void)
{
    printf("Supported workloads:\n");
    printf("  randread   - 4 KiB random read\n");
    printf("  randwrite  - 4 KiB random write\n");
    printf("  seqread    - 128 KiB sequential read\n");
    printf("  seqwrite   - 128 KiB sequential write\n");
    printf("  mixed      - 4 KiB 70/30 random read/write\n");
}
