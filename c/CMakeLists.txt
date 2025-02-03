cmake_minimum_required(VERSION 3.15)
project(advanced_image_optimizer 
    VERSION 3.0.0
    DESCRIPTION "Advanced Image Optimizer - High Performance C Implementation"
    LANGUAGES C CXX CUDA
)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build test suite" ON)
option(USE_GPU "Enable GPU acceleration" ON)
option(USE_SIMD "Enable SIMD optimizations" ON)
option(USE_AI "Enable AI features" ON)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Set CUDA architectures if using GPU
if(USE_GPU)
    set(CMAKE_CUDA_ARCHITECTURES 60 70 75 80 86)
endif()

# Find required packages
find_package(OpenCV REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)
find_package(OpenMP REQUIRED)

if(USE_GPU)
    find_package(CUDA REQUIRED)
    find_package(CUDAToolkit REQUIRED)
endif()

if(USE_AI)
    find_package(TensorRT REQUIRED)
    find_package(ONNXRuntime REQUIRED)
endif()

# Add subdirectories
add_subdirectory(src)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Configure library
add_library(optimizer
    src/core/optimizer.c
    src/core/image_loader.c
    src/core/metadata.c
    src/core/compression.c
    src/core/color_convert.c
    src/core/resize.c
    $<$<BOOL:${USE_GPU}>:src/gpu/cuda_kernels.cu>
    $<$<BOOL:${USE_GPU}>:src/gpu/gpu_manager.c>
    $<$<BOOL:${USE_AI}>:src/ai/ai_manager.c>
    $<$<BOOL:${USE_AI}>:src/ai/models.c>
)

# Set include directories
target_include_directories(optimizer
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${OpenCV_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIR}
        ${PNG_INCLUDE_DIR}
        ${TIFF_INCLUDE_DIR}
)

# Link dependencies
target_link_libraries(optimizer
    PRIVATE
        ${OpenCV_LIBS}
        ${JPEG_LIBRARIES}
        ${PNG_LIBRARIES}
        ${TIFF_LIBRARIES}
        OpenMP::OpenMP_C
)

if(USE_GPU)
    target_link_libraries(optimizer
        PRIVATE
            CUDA::cudart
            CUDA::cuda_driver
            CUDA::nppc
            CUDA::nppig
    )
endif()

if(USE_AI)
    target_link_libraries(optimizer
        PRIVATE
            TensorRT::TensorRT
            ONNXRuntime::ONNXRuntime
    )
endif()

# Compile definitions
target_compile_definitions(optimizer
    PRIVATE
        $<$<BOOL:${USE_GPU}>:OPTIMIZER_USE_GPU>
        $<$<BOOL:${USE_SIMD}>:OPTIMIZER_USE_SIMD>
        $<$<BOOL:${USE_AI}>:OPTIMIZER_USE_AI>
)

# Compile options
target_compile_options(optimizer
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>
        $<$<C_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>
        $<$<C_COMPILER_ID:MSVC>:/W4>
        $<$<BOOL:${USE_SIMD}>:-march=native>
)

# Installation
include(GNUInstallDirs)
install(TARGETS optimizer
    EXPORT optimizer-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

install(
    EXPORT optimizer-targets
    FILE OptimizerTargets.cmake
    NAMESPACE Optimizer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Optimizer
)

# Package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "OptimizerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/OptimizerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/OptimizerConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Optimizer
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/OptimizerConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/OptimizerConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Optimizer
) 