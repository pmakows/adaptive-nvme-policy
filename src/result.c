#include "result.h"
#include "cJSON.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTES_PER_MIB (1024.0 * 1024.0)
#define NANOSECONDS_PER_MICROSECOND 1000.0

static char *read_entire_file(const char *path)
{
    FILE *file = NULL;
    char *buffer = NULL;
    long file_size = 0;
    size_t bytes_read = 0;

    file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }

    file_size = ftell(file);
    if (file_size <= 0) {
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    buffer = malloc((size_t)file_size + 1U);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    bytes_read = fread(buffer, 1U, (size_t)file_size, file);
    fclose(file);

    if (bytes_read != (size_t)file_size) {
        free(buffer);
        return NULL;
    }

    buffer[file_size] = '\0';

    return buffer;
}

static int get_number(
    const cJSON *object,
    const char *field_name,
    double *value)
{
    const cJSON *item = NULL;

    if (object == NULL || field_name == NULL || value == NULL) {
        return 0;
    }

    item = cJSON_GetObjectItemCaseSensitive(object, field_name);

    if (!cJSON_IsNumber(item)) {
        return 0;
    }

    *value = item->valuedouble;

    return 1;
}

static int parse_io_direction(
    const cJSON *job,
    const char *direction_name,
    double *iops,
    double *bandwidth_mib,
    double *mean_latency_us,
    double *p99_latency_us)
{
    const cJSON *direction = NULL;
    const cJSON *clat_ns = NULL;
    const cJSON *percentile = NULL;

    double bandwidth_bytes = 0.0;
    double mean_latency_ns = 0.0;
    double p99_latency_ns = 0.0;

    if (job == NULL ||
        direction_name == NULL ||
        iops == NULL ||
        bandwidth_mib == NULL ||
        mean_latency_us == NULL ||
        p99_latency_us == NULL) {
        return 0;
    }

    direction = cJSON_GetObjectItemCaseSensitive(job, direction_name);
    if (!cJSON_IsObject(direction)) {
        return 0;
    }

    if (!get_number(direction, "iops", iops)) {
        return 0;
    }

    if (!get_number(direction, "bw_bytes", &bandwidth_bytes)) {
        return 0;
    }

    clat_ns = cJSON_GetObjectItemCaseSensitive(direction, "clat_ns");
    if (!cJSON_IsObject(clat_ns)) {
        return 0;
    }

    if (!get_number(clat_ns, "mean", &mean_latency_ns)) {
        return 0;
    }

    percentile = cJSON_GetObjectItemCaseSensitive(clat_ns, "percentile");
    if (!cJSON_IsObject(percentile)) {
        return 0;
    }

    if (!get_number(percentile, "99.000000", &p99_latency_ns)) {
        return 0;
    }

    *bandwidth_mib = bandwidth_bytes / BYTES_PER_MIB;
    *mean_latency_us = mean_latency_ns / NANOSECONDS_PER_MICROSECOND;
    *p99_latency_us = p99_latency_ns / NANOSECONDS_PER_MICROSECOND;

    return 1;
}

result_status_t result_parse_file(
    const char *json_path,
    benchmark_result_t *result)
{
    char *json_text = NULL;

    cJSON *root = NULL;
    cJSON *jobs = NULL;
    cJSON *job = NULL;

    result_status_t status = RESULT_SUCCESS;

    if (json_path == NULL || result == NULL) {
        return RESULT_ERROR_INVALID_ARGUMENT;
    }

    memset(result, 0, sizeof(*result));

    json_text = read_entire_file(json_path);
    if (json_text == NULL) {
        return RESULT_ERROR_FILE_READ;
    }

    root = cJSON_Parse(json_text);
    if (root == NULL) {
        status = RESULT_ERROR_JSON_PARSE;
        goto cleanup;
    }

    jobs = cJSON_GetObjectItemCaseSensitive(root, "jobs");
    if (!cJSON_IsArray(jobs) || cJSON_GetArraySize(jobs) < 1) {
        status = RESULT_ERROR_MISSING_DATA;
        goto cleanup;
    }

    job = cJSON_GetArrayItem(jobs, 0);
    if (!cJSON_IsObject(job)) {
        status = RESULT_ERROR_MISSING_DATA;
        goto cleanup;
    }

    if (!parse_io_direction(
            job,
            "read",
            &result->read_iops,
            &result->read_bandwidth_mib,
            &result->read_mean_latency_us,
            &result->read_p99_latency_us)) {
        status = RESULT_ERROR_MISSING_DATA;
        goto cleanup;
    }

    if (!parse_io_direction(
            job,
            "write",
            &result->write_iops,
            &result->write_bandwidth_mib,
            &result->write_mean_latency_us,
            &result->write_p99_latency_us)) {
        status = RESULT_ERROR_MISSING_DATA;
        goto cleanup;
    }

    if (!get_number(job, "usr_cpu", &result->cpu_user_percent)) {
        status = RESULT_ERROR_MISSING_DATA;
        goto cleanup;
    }

    if (!get_number(job, "sys_cpu", &result->cpu_system_percent)) {
        status = RESULT_ERROR_MISSING_DATA;
        goto cleanup;
    }

cleanup:
    cJSON_Delete(root);
    free(json_text);

    return status;
}

void result_print(const benchmark_result_t *result)
{
    if (result == NULL) {
        return;
    }

    printf("\n");
    printf("Benchmark Summary\n");
    printf("--------------------------------------------------\n");

    printf("Read IOPS:              %.2f\n", result->read_iops);
    printf("Write IOPS:             %.2f\n", result->write_iops);

    printf("\n");

    printf(
        "Read bandwidth:         %.2f MiB/s\n",
        result->read_bandwidth_mib);

    printf(
        "Write bandwidth:        %.2f MiB/s\n",
        result->write_bandwidth_mib);

    printf("\n");

    printf(
        "Read mean latency:      %.2f us\n",
        result->read_mean_latency_us);

    printf(
        "Write mean latency:     %.2f us\n",
        result->write_mean_latency_us);

    printf(
        "Read p99 latency:       %.2f us\n",
        result->read_p99_latency_us);

    printf(
        "Write p99 latency:      %.2f us\n",
        result->write_p99_latency_us);

    printf("\n");

    printf(
        "CPU user:               %.2f %%\n",
        result->cpu_user_percent);

    printf(
        "CPU system:             %.2f %%\n",
        result->cpu_system_percent);

    printf("--------------------------------------------------\n");
}

const char *result_status_string(result_status_t status)
{
    switch (status) {
    case RESULT_SUCCESS:
        return "success";

    case RESULT_ERROR_INVALID_ARGUMENT:
        return "invalid argument";

    case RESULT_ERROR_FILE_READ:
        return "failed to read result file";

    case RESULT_ERROR_JSON_PARSE:
        return "invalid JSON";

    case RESULT_ERROR_MISSING_DATA:
        return "required fio data is missing";

    default:
        return "unknown result parser error";
    }
}
