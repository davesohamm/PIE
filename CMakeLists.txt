cmake_minimum_required(VERSION 3.15)
project(PIE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

# Find LLVM
find_package(LLVM 14 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# Find OpenCL (optional)
find_package(OpenCL)
if(OpenCL_FOUND)
    message(STATUS "OpenCL found: ${OpenCL_VERSION_STRING}")
    add_definitions(-DHAVE_OPENCL)
else()
    message(WARNING "OpenCL not found. GPU features will be disabled.")
endif()

# Find threads
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# Source files
set(PIE_CORE_SOURCES
    src/core/profiler.cpp
    src/core/analyzer.cpp
    src/core/optimizer.cpp
    src/core/config.cpp
    src/core/logger.cpp
)

set(PIE_WORKLOAD_SOURCES
    src/workloads/matmul.cpp
    src/workloads/vector_ops.cpp
    src/workloads/memory_bound.cpp
    src/workloads/branch_heavy.cpp
    src/workloads/reduction.cpp
    src/workloads/workload_factory.cpp
)

set(PIE_LLVM_SOURCES
    src/llvm/CacheTilingPass.cpp
    src/llvm/SIMDVectorizationPass.cpp
    src/llvm/PassManager.cpp
)

set(PIE_GPU_SOURCES
    src/gpu/executor.cpp
    src/gpu/kernels.cpp
)

set(PIE_UTIL_SOURCES
    src/utils/timer.cpp
    src/utils/system_info.cpp
    src/utils/file_utils.cpp
)

# Main executable
add_executable(pie
    src/main.cpp
    ${PIE_CORE_SOURCES}
    ${PIE_WORKLOAD_SOURCES}
    ${PIE_UTIL_SOURCES}
)

# Link LLVM libraries
llvm_map_components_to_libnames(llvm_libs
    core
    support
    irreader
    analysis
    transformutils
    scalaropts
    vectorize
    instcombine
    ipo
    passes
)

target_link_libraries(pie PRIVATE
    ${llvm_libs}
    Threads::Threads
    stdc++fs
)

# LLVM optimization passes (shared library)
add_library(PIEPasses MODULE
    ${PIE_LLVM_SOURCES}
)

target_link_libraries(PIEPasses PRIVATE
    ${llvm_libs}
)

set_target_properties(PIEPasses PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

# GPU executor (if OpenCL is available)
if(OpenCL_FOUND)
    add_library(PIE_GPU SHARED
        ${PIE_GPU_SOURCES}
    )
    
    target_link_libraries(PIE_GPU PRIVATE
        ${OpenCL_LIBRARIES}
    )
    
    target_include_directories(PIE_GPU PRIVATE
        ${OpenCL_INCLUDE_DIRS}
    )
    
    target_link_libraries(pie PRIVATE PIE_GPU)
endif()

# Testing
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS pie DESTINATION bin)
install(TARGETS PIEPasses DESTINATION lib/pie)
if(OpenCL_FOUND)
    install(TARGETS PIE_GPU DESTINATION lib/pie)
endif()

# Copy OpenCL kernels
install(DIRECTORY src/gpu/kernels/ DESTINATION share/pie/kernels
        FILES_MATCHING PATTERN "*.cl")

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "PIE Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  LLVM version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "  OpenCL support: ${OpenCL_FOUND}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")

