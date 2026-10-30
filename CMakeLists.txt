cmake_minimum_required(VERSION 3.20)
project(Compiler2026 C CXX)

option(SYC_ENABLE_RESEARCH_PASSES
       "Enable non-submission research optimization switches" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -O2)

file(GLOB_RECURSE COMPILER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# strict 提交构建不编译仅供研究使用的独立变换实现。研究构建通过显式选项
# 恢复这些源文件和对应入口，避免只在运行时关闭却仍把实现带入提交二进制。
set(RESEARCH_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/ast_matmul_interchange.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/cross_call_array_summary.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/demand_driven_copy_transform.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/linear_recurrence_fusion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/loop_fusion.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/loop_interchange.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/loop_scalar_accumulator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/reflective_max_simplify.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/radix_sort_lowering.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/hir/transpose_view_forwarding.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core_ir/passes/jump_threading.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core_ir/passes/loop_continue_bound.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core_ir/passes/loop_zero_coefficient_specialization.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core_ir/passes/producer_map.cpp")

set(STRICT_SOURCES ${COMPILER_SOURCES})
list(REMOVE_ITEM STRICT_SOURCES ${RESEARCH_SOURCES})
if(NOT SYC_ENABLE_RESEARCH_PASSES)
    set(COMPILER_SOURCES ${STRICT_SOURCES})
endif()

add_executable(compiler ${COMPILER_SOURCES})
if(SYC_ENABLE_RESEARCH_PASSES)
    target_compile_definitions(compiler PRIVATE SYC_ENABLE_RESEARCH_PASSES=1)
endif()
target_include_directories(compiler PRIVATE
    src
    src/frontend
    src/backend
    src/analysis
    src/core_ir
    src/core_ir/ir
    src/core_ir/lowering
    src/core_ir/passes
    src/hir
    src/backend_mir
    src/rewrite
)

# 比赛平台会把编译器生成的汇编与平台 SysY 运行时链接。本地运行时只用于
# QEMU 回归，可按需提供，不属于提交源码。
set(SYC_LOCAL_RUNTIME_SOURCE "" CACHE FILEPATH
    "Optional local SysY runtime source used only for regression tests")
if(SYC_LOCAL_RUNTIME_SOURCE)
    if(NOT EXISTS "${SYC_LOCAL_RUNTIME_SOURCE}")
        message(FATAL_ERROR
            "SYC_LOCAL_RUNTIME_SOURCE does not exist: ${SYC_LOCAL_RUNTIME_SOURCE}")
    endif()
    find_program(RISCV_GCC riscv64-linux-gnu-gcc REQUIRED)
    find_program(RISCV_AR riscv64-linux-gnu-ar REQUIRED)

    set(RUNTIME_OBJ "${CMAKE_BINARY_DIR}/libsysy_runtime.o")
    set(RUNTIME_LIB "${CMAKE_BINARY_DIR}/libsysy_runtime.a")

    add_custom_command(
        OUTPUT "${RUNTIME_LIB}"
        BYPRODUCTS "${RUNTIME_OBJ}"
        COMMAND "${RISCV_GCC}" -march=rv64gc -mcmodel=medany
                -c "${SYC_LOCAL_RUNTIME_SOURCE}"
                -o "${RUNTIME_OBJ}"
        COMMAND "${RISCV_AR}" rcs "${RUNTIME_LIB}" "${RUNTIME_OBJ}"
        DEPENDS "${SYC_LOCAL_RUNTIME_SOURCE}"
        VERBATIM
    )

    add_custom_target(sysy_runtime ALL DEPENDS "${RUNTIME_LIB}")
else()
    message(STATUS
        "No local SysY runtime configured; build compiler only."
        " Pass -DSYC_LOCAL_RUNTIME_SOURCE=<path> for local QEMU tests.")
endif()
