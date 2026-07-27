# Adaptive NVMe Policy Engine

A C-based storage performance exploration framework built on top of fio.

The project automatically executes NVMe benchmarks, collects performance metrics,
and recommends the optimal Queue Depth using configurable decision policies.

Current implementation focuses on Linux NVMe devices through fio.
Future milestones will extend the framework with multidimensional experiments and SPDK support.

---

# Motivation

Modern NVMe SSDs expose massive parallelism through multiple queues and NAND channels.

Finding the optimal Queue Depth is not always obvious:

- low Queue Depth → low latency
- high Queue Depth → high throughput
- excessive Queue Depth → diminishing returns and increased latency

This project automates the exploration process and recommends the best operating point.

---

# Features

## Benchmark Execution

- Execute fio workloads
- Automatic JSON output
- Configurable target device
- Workload abstraction

---

## Benchmark Analytics

Extracts important performance metrics from fio JSON output:

- Read IOPS
- Write IOPS
- Read Bandwidth
- Write Bandwidth
- Average Latency
- P99 Latency
- CPU Usage

---

## Experiment Engine

Automatically evaluates multiple Queue Depth values.

Current sweep:

```
1
2
4
8
16
32
```

Produces:

- JSON benchmark results
- CSV summary
- Console report

---

## Adaptive Policy Engine

Selects the optimal Queue Depth according to different optimization goals.

Supported policies:

### max-iops

Selects the configuration with the highest throughput.

### min-latency

Selects the configuration with the lowest latency.

### balanced

Uses a weighted heuristic that balances throughput and latency.

---

# Project Architecture

```
                 +----------------+
                 |    Workload    |
                 +--------+-------+
                          |
                          v
                 +----------------+
                 | Experiment     |
                 | Engine         |
                 +--------+-------+
                          |
                          v
                 +----------------+
                 | Benchmark      |
                 | Runner         |
                 +--------+-------+
                          |
                          v
                       fio JSON
                          |
                          v
                 +----------------+
                 | Result Parser  |
                 +--------+-------+
                          |
                          v
                 benchmark_result_t
                          |
                          v
                 +----------------+
                 | Policy Engine  |
                 +--------+-------+
                          |
                          v
              Recommended Queue Depth
```

---

# Current CLI

Run a single benchmark

```bash
./ssd_policy_engine \
    --workload randread \
    --target /dev/nvme0n1 \
    --output result.json
```

Run Queue Depth exploration

```bash
./ssd_policy_engine \
    --experiment \
    --policy balanced \
    --workload randread \
    --target /dev/nvme0n1
```

Available policies

```
max-iops
min-latency
balanced
```

---

# Example Output

```
Queue Depth Sweep Summary

QD     IOPS       Latency(us)

1      4584       457
2      7900       514
4      16000      641
8      21065      774
16     41466      766
32     53323      1057

Policy Recommendation

Policy: balanced

Selected Queue Depth: 16
```

---

# Repository Structure

```
adaptive-nvme-policy/

include/

    experiment.h
    policy.h
    result.h
    runner.h
    workload.h

src/

    experiment.c
    policy.c
    result.c
    runner.c
    workload.c

third_party/

    cJSON/

main.c
Makefile
README.md
```

---

# Milestones

## ✅ M1

Benchmark Execution Layer

- fio integration
- workload abstraction
- benchmark execution

---

## ✅ M2

Benchmark Analytics

- JSON parser
- benchmark metrics extraction
- structured benchmark results

---

## ✅ M3

Experiment Engine

- Queue Depth sweep
- CSV generation
- automated benchmark execution

---

## ✅ M4

Adaptive Policy Engine

- max-iops policy
- min-latency policy
- balanced policy
- automatic Queue Depth recommendation

---

## 🚧 M5 (planned)

Parameter Space Exploration

- Block Size sweep
- NumJobs sweep
- Read/Write ratio sweep
- Multiple benchmark repetitions
- Statistical aggregation

---

## 🚧 M6 (planned)

Storage Workload Advisor

- Constraint-based optimization
- Workload recommendation
- Multi-objective decision engine
- Storage tuning advisor

---

# Technologies

- C17
- fio
- cJSON
- Linux
- NVMe

Future:

- SPDK
- io_uring
- DPDK

---

# License

MIT License
