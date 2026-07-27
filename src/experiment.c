#include "experiment.h"

#include "runner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const unsigned QUEUE_DEPTHS[] = {
    1U,
    2U,
    4U,
    8U,
    16U,
    32U
};

static const size_t QUEUE_DEPTH_COUNT =
    sizeof(QUEUE_DEPTHS) / sizeof(QUEUE_DEPTHS[0]);

static int build_result_path(
    char *buffer,
    size_t buffer_size,
    const char *results_directory,
    const char *workload_name,
    unsigned queue_depth)
{
    int written;

    if (buffer == NULL ||
        buffer_size == 0U ||
        results_directory == NULL ||
        workload_name == NULL) {
        return -1;
    }

    written = snprintf(
        buffer,
        buffer_size,
        "%s/%s_qd%u.json",
        results_directory,
        workload_name,
        queue_depth
    );

    if (written < 0 || (size_t)written >= buffer_size) {
        return -1;
    }

    return 0;
}

int experiment_run_queue_depth_sweep(
    const workload_t *base_workload,
    const char *target_path,
    const char *results_directory,
    experiment_series_t *series)
{
    workload_t workload;
    char result_path[512];

    if (base_workload == NULL ||
        target_path == NULL ||
        results_directory == NULL ||
        series == NULL) {
        return -1;
    }

    series->items = calloc(
        QUEUE_DEPTH_COUNT,
        sizeof(*series->items)
    );

    if (series->items == NULL) {
        return -1;
    }

    series->count = QUEUE_DEPTH_COUNT;

    for (size_t index = 0U;
         index < QUEUE_DEPTH_COUNT;
         ++index) {

        result_status_t parse_status;

        workload = *base_workload;
        workload.queue_depth = QUEUE_DEPTHS[index];

        series->items[index].queue_depth =
            workload.queue_depth;

        if (build_result_path(
                result_path,
                sizeof(result_path),
                results_directory,
                workload.name,
                workload.queue_depth) != 0) {
            experiment_series_free(series);
            return -1;
        }

        printf(
            "\nRunning experiment %zu/%zu: queue depth = %u\n",
            index + 1U,
            QUEUE_DEPTH_COUNT,
            workload.queue_depth
        );

        if (runner_execute(
                &workload,
                target_path,
                result_path) != 0) {
            fprintf(
                stderr,
                "Error: benchmark failed for queue depth %u.\n",
                workload.queue_depth
            );
            experiment_series_free(series);
            return -1;
        }

        parse_status = result_parse_file(
            result_path,
            &series->items[index].result
        );

        if (parse_status != RESULT_SUCCESS) {
            fprintf(
                stderr,
                "Error: failed to parse result for queue depth %u: %s\n",
                workload.queue_depth,
                result_status_string(parse_status)
            );
            experiment_series_free(series);
            return -1;
        }
    }

    return 0;
}

void experiment_print_summary(
    const experiment_series_t *series)
{
    if (series == NULL || series->items == NULL) {
        return;
    }

    printf("\nQueue Depth Sweep Summary\n");
    printf(
        "-------------------------------------------------------------------------------\n"
    );
    printf(
        "%-6s %-12s %-12s %-14s %-14s %-10s %-10s\n",
        "QD",
        "Read IOPS",
        "Write IOPS",
        "Read mean us",
        "Read p99 us",
        "CPU usr",
        "CPU sys"
    );
    printf(
        "-------------------------------------------------------------------------------\n"
    );

    for (size_t index = 0U;
         index < series->count;
         ++index) {

        const experiment_result_t *item =
            &series->items[index];

        printf(
            "%-6u %-12.2f %-12.2f %-14.2f %-14.2f %-10.2f %-10.2f\n",
            item->queue_depth,
            item->result.read_iops,
            item->result.write_iops,
            item->result.read_mean_latency_us,
            item->result.read_p99_latency_us,
            item->result.cpu_user_percent,
            item->result.cpu_system_percent
        );
    }

    printf(
        "-------------------------------------------------------------------------------\n"
    );
}

int experiment_write_csv(
    const char *csv_path,
    const workload_t *base_workload,
    const experiment_series_t *series)
{
    FILE *file;

    if (csv_path == NULL ||
        base_workload == NULL ||
        series == NULL ||
        series->items == NULL) {
        return -1;
    }

    file = fopen(csv_path, "w");
    if (file == NULL) {
        return -1;
    }

    fprintf(
        file,
        "workload,queue_depth,jobs,block_size_bytes,"
        "read_iops,write_iops,"
        "read_bandwidth_mib,write_bandwidth_mib,"
        "read_mean_latency_us,write_mean_latency_us,"
        "read_p99_latency_us,write_p99_latency_us,"
        "cpu_user_percent,cpu_system_percent\n"
    );

    for (size_t index = 0U;
         index < series->count;
         ++index) {

        const experiment_result_t *item =
            &series->items[index];

        fprintf(
            file,
            "%s,%u,%u,%u,"
            "%.2f,%.2f,"
            "%.2f,%.2f,"
            "%.2f,%.2f,"
            "%.2f,%.2f,"
            "%.2f,%.2f\n",
            base_workload->name,
            item->queue_depth,
            base_workload->jobs,
            base_workload->block_size_bytes,
            item->result.read_iops,
            item->result.write_iops,
            item->result.read_bandwidth_mib,
            item->result.write_bandwidth_mib,
            item->result.read_mean_latency_us,
            item->result.write_mean_latency_us,
            item->result.read_p99_latency_us,
            item->result.write_p99_latency_us,
            item->result.cpu_user_percent,
            item->result.cpu_system_percent
        );
    }

    fclose(file);

    return 0;
}

void experiment_series_free(
    experiment_series_t *series)
{
    if (series == NULL) {
        return;
    }

    free(series->items);
    series->items = NULL;
    series->count = 0U;
}
