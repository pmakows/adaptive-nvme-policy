cmake_minimum_required(VERSION 3.16)

project(adaptive_nvme_policy C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(ssd_policy_engine
    src/main.c
    src/workload.c
    src/runner.c
    src/result.c
    src/experiment.c
    src/policy.c
    third_party/cjson/cJSON.c
)

target_include_directories(ssd_policy_engine
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/third_party/cjson
)

target_compile_options(ssd_policy_engine
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
)
