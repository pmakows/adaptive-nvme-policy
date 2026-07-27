#ifndef RESULT_H
#define RESULT_H

typedef struct {
    double read_iops;
    double write_iops;

    double read_bandwidth_mib;
    double write_bandwidth_mib;

    double read_mean_latency_us;
    double write_mean_latency_us;

    double read_p99_latency_us;
    double write_p99_latency_us;

    double cpu_user_percent;
    double cpu_system_percent;
} benchmark_result_t;

typedef enum {
    RESULT_SUCCESS = 0,
    RESULT_ERROR_INVALID_ARGUMENT = -1,
    RESULT_ERROR_FILE_READ = -2,
    RESULT_ERROR_JSON_PARSE = -3,
    RESULT_ERROR_MISSING_DATA = -4
} result_status_t;

result_status_t result_parse_file(
    const char *json_path,
    benchmark_result_t *result
);

void result_print(const benchmark_result_t *result);

const char *result_status_string(result_status_t status);

#endif
