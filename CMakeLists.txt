cmake_minimum_required(VERSION 3.16)

project(adaptive_nvme_policy
    VERSION 0.1.0
    DESCRIPTION "Adaptive NVMe I/O Policy Engine"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(ssd_policy_engine
    src/main.c
    src/workload.c
    src/runner.c
)

target_include_directories(ssd_policy_engine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ssd_policy_engine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()
