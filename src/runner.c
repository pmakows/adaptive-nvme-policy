#include "runner.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define COMMAND_BUFFER_SIZE 2048U

int runner_execute(
    const workload_t *workload,
    const char *target,
    const char *result_path)
{
    char command[COMMAND_BUFFER_SIZE];
    int written;
    int status;

    if (workload == NULL || target == NULL || result_path == NULL) {
        fprintf(stderr, "runner_execute: invalid argument\n");
        return -1;
    }

    if (workload->type == WORKLOAD_MIXED) {
        written = snprintf(
            command,
            sizeof(command),
            "fio "
            "--name=\"%s\" "
            "--filename=\"%s\" "
            "--rw=%s "
            "--rwmixread=%u "
            "--bs=%u "
            "--iodepth=%u "
            "--numjobs=%u "
            "--ioengine=io_uring "
            "--direct=1 "
            "--time_based=1 "
            "--runtime=%u "
            "--ramp_time=2 "
            "--size=1G "
            "--group_reporting=1 "
            "--output-format=json "
            "--output=\"%s\"",
            workload_type_to_string(workload->type),
            target,
            workload->fio_rw,
            workload->read_percentage,
            workload->block_size_bytes,
            workload->queue_depth,
            workload->jobs,
            workload->runtime_seconds,
            result_path
        );
    } else {
        written = snprintf(
            command,
            sizeof(command),
            "fio "
            "--name=\"%s\" "
            "--filename=\"%s\" "
            "--rw=%s "
            "--bs=%u "
            "--iodepth=%u "
            "--numjobs=%u "
            "--ioengine=io_uring "
            "--direct=1 "
            "--time_based=1 "
            "--runtime=%u "
            "--ramp_time=2 "
            "--size=1G "
            "--group_reporting=1 "
            "--output-format=json "
            "--output=\"%s\"",
            workload_type_to_string(workload->type),
            target,
            workload->fio_rw,
            workload->block_size_bytes,
            workload->queue_depth,
            workload->jobs,
            workload->runtime_seconds,
            result_path
        );
    }

    if (written < 0 || (size_t)written >= sizeof(command)) {
        fprintf(stderr, "runner_execute: command buffer too small\n");
        return -1;
    }

    printf("\nStarting fio benchmark\n");
    printf("----------------------\n");
    printf("Target : %s\n", target);
    printf("Output : %s\n\n", result_path);

    printf("Command:\n%s\n\n", command);

    status = system(command);

    if (status == -1) {
        perror("system");
        return -1;
    }

    if (!WIFEXITED(status)) {
        fprintf(stderr, "fio terminated abnormally\n");
        return -1;
    }

    if (WEXITSTATUS(status) != 0) {
        fprintf(
            stderr,
            "fio failed with exit code %d\n",
            WEXITSTATUS(status)
        );
        return -1;
    }

    printf("fio benchmark completed successfully\n");

    return 0;
}
