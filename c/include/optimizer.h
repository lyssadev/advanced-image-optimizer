#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version information
#define OPTIMIZER_VERSION_MAJOR 3
#define OPTIMIZER_VERSION_MINOR 0
#define OPTIMIZER_VERSION_PATCH 0

// Error codes
typedef enum {
    OPTIMIZER_SUCCESS = 0,
    OPTIMIZER_ERROR_INVALID_PARAM = -1,
    OPTIMIZER_ERROR_FILE_IO = -2,
    OPTIMIZER_ERROR_MEMORY = -3,
    OPTIMIZER_ERROR_UNSUPPORTED_FORMAT = -4,
    OPTIMIZER_ERROR_GPU = -5,
    OPTIMIZER_ERROR_THREAD = -6
} optimizer_error_t;

// Image formats
typedef enum {
    OPTIMIZER_FORMAT_JPEG,
    OPTIMIZER_FORMAT_PNG,
    OPTIMIZER_FORMAT_WEBP,
    OPTIMIZER_FORMAT_AVIF,
    OPTIMIZER_FORMAT_HEIC
} optimizer_format_t;

// Color spaces
typedef enum {
    OPTIMIZER_COLORSPACE_RGB,
    OPTIMIZER_COLORSPACE_RGBA,
    OPTIMIZER_COLORSPACE_GRAY,
    OPTIMIZER_COLORSPACE_CMYK
} optimizer_colorspace_t;

// Compression levels
typedef enum {
    OPTIMIZER_COMPRESS_NONE = 0,
    OPTIMIZER_COMPRESS_FAST = 1,
    OPTIMIZER_COMPRESS_BALANCED = 2,
    OPTIMIZER_COMPRESS_MAX = 3
} optimizer_compress_level_t;

// Image metadata
typedef struct {
    char* exif_data;
    size_t exif_size;
    char* icc_profile;
    size_t icc_size;
    char* xmp_data;
    size_t xmp_size;
} optimizer_metadata_t;

// Processing options
typedef struct {
    optimizer_format_t output_format;
    optimizer_compress_level_t compress_level;
    optimizer_colorspace_t colorspace;
    uint32_t quality;
    uint32_t max_width;
    uint32_t max_height;
    bool preserve_metadata;
    bool use_gpu;
    uint32_t thread_count;
    bool enable_preview;
} optimizer_options_t;

// Image information
typedef struct {
    uint32_t width;
    uint32_t height;
    optimizer_colorspace_t colorspace;
    uint32_t bits_per_pixel;
    size_t file_size;
    optimizer_format_t format;
    optimizer_metadata_t metadata;
} optimizer_image_info_t;

// Progress callback
typedef void (*optimizer_progress_callback)(float progress, void* user_data);

// Preview callback
typedef void (*optimizer_preview_callback)(const uint8_t* buffer, uint32_t width, 
                                        uint32_t height, void* user_data);

// Core functions
optimizer_error_t optimizer_init(void);
void optimizer_cleanup(void);
const char* optimizer_version_string(void);
const char* optimizer_error_string(optimizer_error_t error);

// Image processing functions
optimizer_error_t optimizer_process_file(
    const char* input_path,
    const char* output_path,
    const optimizer_options_t* options,
    optimizer_progress_callback progress_cb,
    optimizer_preview_callback preview_cb,
    void* user_data
);

optimizer_error_t optimizer_process_buffer(
    const uint8_t* input_buffer,
    size_t input_size,
    uint8_t** output_buffer,
    size_t* output_size,
    const optimizer_options_t* options,
    optimizer_progress_callback progress_cb,
    optimizer_preview_callback preview_cb,
    void* user_data
);

// Information functions
optimizer_error_t optimizer_get_image_info(
    const char* path,
    optimizer_image_info_t* info
);

optimizer_error_t optimizer_get_supported_formats(
    optimizer_format_t** formats,
    size_t* count
);

// GPU functions
optimizer_error_t optimizer_get_gpu_devices(
    char*** device_names,
    size_t* count
);

optimizer_error_t optimizer_set_gpu_device(
    size_t device_index
);

// Memory management
void optimizer_free_buffer(uint8_t* buffer);
void optimizer_free_image_info(optimizer_image_info_t* info);
void optimizer_free_device_names(char** device_names, size_t count);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZER_H 