#include "experiment.h"
#include "policy.h"
#include "result.h"
#include "runner.h"
#include "workload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_NAME "ssd_policy_engine"
#define APP_VERSION "0.4.0"

#define DEFAULT_RESULT_PATH "results/fio_result.json"
#define DEFAULT_RESULTS_DIRECTORY "results"
#define DEFAULT_EXPERIMENT_CSV_PATH "results/experiment.csv"

static void print_supported_policies(void)
{
    printf("Supported policies:\n");
    printf("  max-iops\n");
    printf("  min-latency\n");
    printf("  balanced\n");
}

static void print_usage(const char *program_name)
{
    printf("%s %s\n", APP_NAME, APP_VERSION);
    printf("Adaptive NVMe I/O Policy Engine\n\n");

    printf("Usage:\n");
    printf(
        "  %s --workload <name> --target <path> [--output <path>]\n",
        program_name
    );
    printf(
        "  %s --experiment --policy <name> "
        "--workload <name> --target <path>\n",
        program_name
    );
    printf("  %s --list-workloads\n", program_name);
    printf("  %s --help\n\n", program_name);

    printf("Examples:\n");
    printf(
        "  %s --workload randread "
        "--target ./testdata/benchmark.bin\n",
        program_name
    );
    printf(
        "  %s --workload mixed "
        "--target ./testdata/benchmark.bin "
        "--output results/mixed.json\n",
        program_name
    );
    printf(
        "  %s --experiment "
        "--policy balanced "
        "--workload randread "
        "--target ./testdata/benchmark.bin\n\n",
        program_name
    );

    workload_print_supported();
    printf("\n");
    print_supported_policies();
}

static const char *find_argument_value(
    int argc,
    char *argv[],
    const char *argument_name)
{
    for (int index = 1; index < argc - 1; ++index) {
        if (strcmp(argv[index], argument_name) == 0) {
            return argv[index + 1];
        }
    }

    return NULL;
}

static int has_argument(
    int argc,
    char *argv[],
    const char *argument_name)
{
    for (int index = 1; index < argc; ++index) {
        if (strcmp(argv[index], argument_name) == 0) {
            return 1;
        }
    }

    return 0;
}

static int run_single_benchmark(
    const workload_t *workload,
    const char *target_path,
    const char *result_path)
{
    benchmark_result_t result = {0};
    result_status_t result_status;

    workload_print(workload);

    if (runner_execute(
            workload,
            target_path,
            result_path) != 0) {
        fprintf(stderr, "Error: fio benchmark failed.\n");
        return -1;
    }

    printf("\nResult saved to: %s\n", result_path);

    result_status =
        result_parse_file(result_path, &result);

    if (result_status != RESULT_SUCCESS) {
        fprintf(
            stderr,
            "Error: failed to parse benchmark result: %s\n",
            result_status_string(result_status)
        );
        return -1;
    }

    result_print(&result);

    return 0;
}

static int run_experiment(
    const workload_t *workload,
    const char *target_path,
    policy_type_t policy)
{
    experiment_series_t series = {0};
    policy_decision_t decision = {0};

    printf("\nStarting queue depth sweep\n");
    printf("--------------------------\n");
    printf("Workload:     %s\n", workload->name);
    printf("Target:       %s\n", target_path);
    printf(
        "Policy:       %s\n",
        policy_type_to_string(policy)
    );
    printf("Queue depths: 1, 2, 4, 8, 16, 32\n");

    if (experiment_run_queue_depth_sweep(
            workload,
            target_path,
            DEFAULT_RESULTS_DIRECTORY,
            &series) != 0) {
        fprintf(stderr, "Error: experiment execution failed.\n");
        return -1;
    }

    experiment_print_summary(&series);

    if (experiment_write_csv(
            DEFAULT_EXPERIMENT_CSV_PATH,
            workload,
            &series) != 0) {
        fprintf(
            stderr,
            "Error: failed to write experiment CSV.\n"
        );
        experiment_series_free(&series);
        return -1;
    }

    if (policy_select(
            policy,
            &series,
            &decision) != 0) {
        fprintf(stderr, "Error: policy selection failed.\n");
        experiment_series_free(&series);
        return -1;
    }

    policy_print_decision(&decision);

    printf(
        "\nExperiment CSV saved to: %s\n",
        DEFAULT_EXPERIMENT_CSV_PATH
    );

    experiment_series_free(&series);

    return 0;
}

int main(int argc, char *argv[])
{
    const char *workload_name;
    const char *target_path;
    const char *result_path;
    const char *policy_name;

    workload_type_t workload_type;
    workload_t workload = {0};

    policy_type_t policy;

    if (argc == 1 ||
        has_argument(argc, argv, "--help") ||
        has_argument(argc, argv, "-h")) {
        print_usage(argv[0]);
        return EXIT_SUCCESS;
    }

    if (has_argument(argc, argv, "--list-workloads")) {
        workload_print_supported();
        return EXIT_SUCCESS;
    }

    workload_name =
        find_argument_value(argc, argv, "--workload");

    if (workload_name == NULL) {
        fprintf(
            stderr,
            "Error: missing --workload <name> argument.\n\n"
        );
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    target_path =
        find_argument_value(argc, argv, "--target");

    if (target_path == NULL) {
        fprintf(
            stderr,
            "Error: missing --target <path> argument.\n\n"
        );
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    workload_type =
        workload_type_from_string(workload_name);

    if (workload_type == WORKLOAD_INVALID) {
        fprintf(
            stderr,
            "Error: unsupported workload '%s'.\n\n",
            workload_name
        );
        workload_print_supported();
        return EXIT_FAILURE;
    }

    if (workload_create_default(
            workload_type,
            &workload) != 0) {
        fprintf(
            stderr,
            "Error: failed to create workload configuration.\n"
        );
        return EXIT_FAILURE;
    }

    if (has_argument(argc, argv, "--experiment")) {
        policy_name =
            find_argument_value(argc, argv, "--policy");

        if (policy_name == NULL) {
            fprintf(
                stderr,
                "Error: missing --policy <name> argument.\n\n"
            );
            print_supported_policies();
            return EXIT_FAILURE;
        }

        policy =
            policy_type_from_string(policy_name);

        if (policy == POLICY_INVALID) {
            fprintf(
                stderr,
                "Error: unsupported policy '%s'.\n\n",
                policy_name
            );
            print_supported_policies();
            return EXIT_FAILURE;
        }

        if (run_experiment(
                &workload,
                target_path,
                policy) != 0) {
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    result_path =
        find_argument_value(argc, argv, "--output");

    if (result_path == NULL) {
        result_path = DEFAULT_RESULT_PATH;
    }

    if (run_single_benchmark(
            &workload,
            target_path,
            result_path) != 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
