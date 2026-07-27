#include "runner.h"
#include "workload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_NAME "ssd_policy_engine"
#define APP_VERSION "0.1.0"
#define DEFAULT_RESULT_PATH "results/fio_result.json"

static void print_usage(const char *program_name)
{
    printf("%s %s\n", APP_NAME, APP_VERSION);
    printf("Adaptive NVMe I/O Policy Engine\n\n");

    printf("Usage:\n");
    printf(
        "  %s --workload <name> --target <path> [--output <path>]\n",
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
        "--output results/mixed.json\n\n",
        program_name
    );

    workload_print_supported();
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

int main(int argc, char *argv[])
{
    const char *workload_name;
    const char *target_path;
    const char *result_path;
    workload_type_t workload_type;
    workload_t workload = {0};

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

    result_path =
        find_argument_value(argc, argv, "--output");

    if (result_path == NULL) {
        result_path = DEFAULT_RESULT_PATH;
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

    workload_print(&workload);

    if (runner_execute(
            &workload,
            target_path,
            result_path) != 0) {
        fprintf(stderr, "Error: fio benchmark failed.\n");
        return EXIT_FAILURE;
    }

    printf("\nResult saved to: %s\n", result_path);

    return EXIT_SUCCESS;
}
