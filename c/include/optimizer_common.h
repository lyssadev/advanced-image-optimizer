#ifndef OPTIMIZER_COMMON_H
#define OPTIMIZER_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
    #ifdef OPTIMIZER_EXPORTS
        #define OPTIMIZER_API __declspec(dllexport)
    #else
        #define OPTIMIZER_API __declspec(dllimport)
    #endif
#else
    #define OPTIMIZER_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Common error handling
typedef enum {
    OPTIMIZER_SUCCESS = 0,
    OPTIMIZER_ERROR_INVALID_PARAM = -1,
    OPTIMIZER_ERROR_FILE_IO = -2,
    OPTIMIZER_ERROR_MEMORY = -3,
    OPTIMIZER_ERROR_UNSUPPORTED_FORMAT = -4,
    OPTIMIZER_ERROR_GPU = -5,
    OPTIMIZER_ERROR_THREAD = -6,
    OPTIMIZER_ERROR_SYSTEM = -7,
    OPTIMIZER_ERROR_INITIALIZATION = -8,
    OPTIMIZER_ERROR_NOT_IMPLEMENTED = -9
} optimizer_error_t;

// Common callback types
typedef void (*optimizer_progress_callback)(float progress, void* user_data);
typedef void (*optimizer_log_callback)(const char* message, void* user_data);
typedef void (*optimizer_error_callback)(optimizer_error_t error, const char* message, void* user_data);

// Version information
typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    const char* build_date;
    const char* build_hash;
    const char* platform;
    bool has_gpu_support;
    bool has_ai_support;
} optimizer_version_info_t;

// System information
typedef struct {
    uint32_t cpu_cores;
    uint64_t total_memory;
    uint64_t available_memory;
    const char* os_name;
    const char* os_version;
    const char* cpu_name;
    bool has_avx;
    bool has_avx2;
    bool has_avx512;
    bool has_cuda;
} optimizer_system_info_t;

// Common initialization options
typedef struct {
    bool enable_gpu;
    bool enable_ai;
    uint32_t thread_count;
    const char* temp_dir;
    const char* config_file;
    optimizer_log_callback log_cb;
    optimizer_error_callback error_cb;
    void* user_data;
} optimizer_init_options_t;

// Common functions
OPTIMIZER_API optimizer_error_t optimizer_get_version_info(optimizer_version_info_t* info);
OPTIMIZER_API optimizer_error_t optimizer_get_system_info(optimizer_system_info_t* info);
OPTIMIZER_API const char* optimizer_error_string(optimizer_error_t error);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZER_COMMON_H 