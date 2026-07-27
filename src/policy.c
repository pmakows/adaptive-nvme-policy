#include "policy.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

static double total_iops(
    const experiment_result_t *item)
{
    return item->result.read_iops +
           item->result.write_iops;
}

static double mean_latency_us(
    const experiment_result_t *item)
{
    const double read_latency =
        item->result.read_mean_latency_us;

    const double write_latency =
        item->result.write_mean_latency_us;

    if (read_latency > 0.0 &&
        write_latency > 0.0) {
        return (read_latency + write_latency) / 2.0;
    }

    if (read_latency > 0.0) {
        return read_latency;
    }

    return write_latency;
}

static double p99_latency_us(
    const experiment_result_t *item)
{
    const double read_latency =
        item->result.read_p99_latency_us;

    const double write_latency =
        item->result.write_p99_latency_us;

    if (read_latency > 0.0 &&
        write_latency > 0.0) {
        return (read_latency + write_latency) / 2.0;
    }

    if (read_latency > 0.0) {
        return read_latency;
    }

    return write_latency;
}

const char *policy_type_to_string(
    policy_type_t policy)
{
    switch (policy) {
        case POLICY_MAX_IOPS:
            return "max-iops";

        case POLICY_MIN_LATENCY:
            return "min-latency";

        case POLICY_BALANCED:
            return "balanced";

        case POLICY_INVALID:
        default:
            return "invalid";
    }
}

policy_type_t policy_type_from_string(
    const char *value)
{
    if (value == NULL) {
        return POLICY_INVALID;
    }

    if (strcmp(value, "max-iops") == 0) {
        return POLICY_MAX_IOPS;
    }

    if (strcmp(value, "min-latency") == 0) {
        return POLICY_MIN_LATENCY;
    }

    if (strcmp(value, "balanced") == 0) {
        return POLICY_BALANCED;
    }

    return POLICY_INVALID;
}

static int select_max_iops(
    const experiment_series_t *series,
    policy_decision_t *decision)
{
    size_t selected_index = 0U;
    double best_iops = -1.0;

    for (size_t index = 0U;
         index < series->count;
         ++index) {

        const double current_iops =
            total_iops(&series->items[index]);

        if (current_iops > best_iops) {
            best_iops = current_iops;
            selected_index = index;
        }
    }

    decision->selected_index = selected_index;
    decision->selected_result =
        series->items[selected_index];
    decision->score = best_iops;

    return 0;
}

static int select_min_latency(
    const experiment_series_t *series,
    policy_decision_t *decision)
{
    size_t selected_index = 0U;
    double best_latency = DBL_MAX;

    for (size_t index = 0U;
         index < series->count;
         ++index) {

        const double current_latency =
            mean_latency_us(&series->items[index]);

        if (current_latency > 0.0 &&
            current_latency < best_latency) {
            best_latency = current_latency;
            selected_index = index;
        }
    }

    if (best_latency == DBL_MAX) {
        return -1;
    }

    decision->selected_index = selected_index;
    decision->selected_result =
        series->items[selected_index];
    decision->score = best_latency;

    return 0;
}

static int select_balanced(
    const experiment_series_t *series,
    policy_decision_t *decision)
{
    double max_iops = 0.0;
    double min_p99 = DBL_MAX;
    double max_p99 = 0.0;

    size_t selected_index = 0U;
    double best_score = -DBL_MAX;

    for (size_t index = 0U;
         index < series->count;
         ++index) {

        const double current_iops =
            total_iops(&series->items[index]);

        const double current_p99 =
            p99_latency_us(&series->items[index]);

        if (current_iops > max_iops) {
            max_iops = current_iops;
        }

        if (current_p99 > 0.0 &&
            current_p99 < min_p99) {
            min_p99 = current_p99;
        }

        if (current_p99 > max_p99) {
            max_p99 = current_p99;
        }
    }

    if (max_iops <= 0.0 ||
        min_p99 == DBL_MAX) {
        return -1;
    }

    for (size_t index = 0U;
         index < series->count;
         ++index) {

        const double current_iops =
            total_iops(&series->items[index]);

        const double current_p99 =
            p99_latency_us(&series->items[index]);

        const double normalized_iops =
            current_iops / max_iops;

        double normalized_latency = 1.0;

        if (max_p99 > min_p99) {
            normalized_latency =
                (current_p99 - min_p99) /
                (max_p99 - min_p99);
        }

        /*
         * Higher throughput increases the score.
         * Higher p99 latency decreases the score.
         *
         * Throughput has a slightly higher weight,
         * because the balanced policy should still
         * prefer useful queue parallelism.
         */
        const double score =
            (0.60 * normalized_iops) -
            (0.40 * normalized_latency);

        if (score > best_score) {
            best_score = score;
            selected_index = index;
        }
    }

    decision->selected_index = selected_index;
    decision->selected_result =
        series->items[selected_index];
    decision->score = best_score;

    return 0;
}

int policy_select(
    policy_type_t policy,
    const experiment_series_t *series,
    policy_decision_t *decision)
{
    if (series == NULL ||
        series->items == NULL ||
        series->count == 0U ||
        decision == NULL) {
        return -1;
    }

    *decision = (policy_decision_t){0};
    decision->policy = policy;

    switch (policy) {
        case POLICY_MAX_IOPS:
            return select_max_iops(
                series,
                decision
            );

        case POLICY_MIN_LATENCY:
            return select_min_latency(
                series,
                decision
            );

        case POLICY_BALANCED:
            return select_balanced(
                series,
                decision
            );

        case POLICY_INVALID:
        default:
            return -1;
    }
}

void policy_print_decision(
    const policy_decision_t *decision)
{
    const experiment_result_t *selected;

    if (decision == NULL) {
        return;
    }

    selected = &decision->selected_result;

    printf("\nPolicy Recommendation\n");
    printf("--------------------------------------------------\n");
    printf(
        "Policy:           %s\n",
        policy_type_to_string(decision->policy)
    );
    printf(
        "Selected QD:      %u\n",
        selected->queue_depth
    );
    printf(
        "Read IOPS:        %.2f\n",
        selected->result.read_iops
    );
    printf(
        "Write IOPS:       %.2f\n",
        selected->result.write_iops
    );
    printf(
        "Read mean:        %.2f us\n",
        selected->result.read_mean_latency_us
    );
    printf(
        "Write mean:       %.2f us\n",
        selected->result.write_mean_latency_us
    );
    printf(
        "Read p99:         %.2f us\n",
        selected->result.read_p99_latency_us
    );
    printf(
        "Write p99:        %.2f us\n",
        selected->result.write_p99_latency_us
    );
    printf(
        "Policy score:     %.4f\n",
        decision->score
    );
    printf("--------------------------------------------------\n");
}
