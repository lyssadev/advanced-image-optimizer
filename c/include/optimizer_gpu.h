#ifndef OPTIMIZER_GPU_H
#define OPTIMIZER_GPU_H

#include "optimizer.h"

#ifdef __cplusplus
extern "C" {
#endif

// GPU Device information
typedef struct {
    char* name;
    size_t compute_units;
    size_t clock_frequency;
    size_t memory_size;
    bool supports_fp16;
    bool supports_int8;
    char* vendor;
    char* driver_version;
} optimizer_gpu_device_info_t;

// GPU Memory types
typedef enum {
    OPTIMIZER_GPU_MEMORY_DEFAULT,
    OPTIMIZER_GPU_MEMORY_HOST,
    OPTIMIZER_GPU_MEMORY_DEVICE,
    OPTIMIZER_GPU_MEMORY_UNIFIED
} optimizer_gpu_memory_type_t;

// GPU Buffer
typedef struct {
    void* handle;
    size_t size;
    optimizer_gpu_memory_type_t memory_type;
} optimizer_gpu_buffer_t;

// GPU Kernel types
typedef enum {
    OPTIMIZER_KERNEL_RESIZE,
    OPTIMIZER_KERNEL_COLOR_CONVERT,
    OPTIMIZER_KERNEL_FILTER,
    OPTIMIZER_KERNEL_COMPRESS,
    OPTIMIZER_KERNEL_DECOMPRESS
} optimizer_kernel_type_t;

// GPU initialization and cleanup
optimizer_error_t optimizer_gpu_init(void);
void optimizer_gpu_cleanup(void);

// Device management
optimizer_error_t optimizer_gpu_get_device_count(size_t* count);
optimizer_error_t optimizer_gpu_get_device_info(
    size_t device_index,
    optimizer_gpu_device_info_t* info
);
optimizer_error_t optimizer_gpu_set_device(size_t device_index);

// Memory management
optimizer_error_t optimizer_gpu_allocate(
    size_t size,
    optimizer_gpu_memory_type_t memory_type,
    optimizer_gpu_buffer_t* buffer
);
optimizer_error_t optimizer_gpu_free(optimizer_gpu_buffer_t* buffer);
optimizer_error_t optimizer_gpu_memcpy_host_to_device(
    optimizer_gpu_buffer_t* dst,
    const void* src,
    size_t size
);
optimizer_error_t optimizer_gpu_memcpy_device_to_host(
    void* dst,
    const optimizer_gpu_buffer_t* src,
    size_t size
);

// Kernel management
optimizer_error_t optimizer_gpu_load_kernels(void);
optimizer_error_t optimizer_gpu_unload_kernels(void);

// Image processing operations
optimizer_error_t optimizer_gpu_resize(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    uint32_t input_width,
    uint32_t input_height,
    uint32_t output_width,
    uint32_t output_height,
    optimizer_colorspace_t colorspace
);

optimizer_error_t optimizer_gpu_color_convert(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    uint32_t width,
    uint32_t height,
    optimizer_colorspace_t input_colorspace,
    optimizer_colorspace_t output_colorspace
);

optimizer_error_t optimizer_gpu_compress(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    size_t* output_size,
    uint32_t width,
    uint32_t height,
    optimizer_format_t format,
    uint32_t quality
);

optimizer_error_t optimizer_gpu_decompress(
    const optimizer_gpu_buffer_t* input,
    size_t input_size,
    optimizer_gpu_buffer_t* output,
    uint32_t* width,
    uint32_t* height,
    optimizer_format_t format
);

// Performance monitoring
typedef struct {
    float kernel_time;
    float memory_transfer_time;
    size_t memory_used;
    float gpu_utilization;
    float memory_utilization;
} optimizer_gpu_performance_t;

optimizer_error_t optimizer_gpu_get_performance(
    optimizer_gpu_performance_t* performance
);

// Memory management helpers
void optimizer_gpu_free_device_info(optimizer_gpu_device_info_t* info);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZER_GPU_H 