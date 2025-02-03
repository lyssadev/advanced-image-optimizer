#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include "optimizer.h"
#include "optimizer_gpu.h"
#include "optimizer_ai.h"

// Internal state
static struct {
    bool initialized;
    mtx_t mutex;
    optimizer_format_t supported_formats[16];
    size_t num_supported_formats;
    char version_string[32];
    thread_local optimizer_error_t last_error;
} g_state = {0};

// Error messages
static const char* error_messages[] = {
    "Success",
    "Invalid parameter",
    "File I/O error",
    "Memory allocation error",
    "Unsupported format",
    "GPU error",
    "Thread error"
};

// Helper functions
static bool is_format_supported(optimizer_format_t format) {
    for (size_t i = 0; i < g_state.num_supported_formats; i++) {
        if (g_state.supported_formats[i] == format) {
            return true;
        }
    }
    return false;
}

static void set_error(optimizer_error_t error) {
    g_state.last_error = error;
}

// Core functions implementation
optimizer_error_t optimizer_init(void) {
    if (g_state.initialized) {
        return OPTIMIZER_SUCCESS;
    }

    // Initialize mutex
    if (mtx_init(&g_state.mutex, mtx_plain) != thrd_success) {
        return OPTIMIZER_ERROR_THREAD;
    }

    // Initialize GPU subsystem
    optimizer_error_t err = optimizer_gpu_init();
    if (err != OPTIMIZER_SUCCESS) {
        mtx_destroy(&g_state.mutex);
        return err;
    }

    // Initialize AI subsystem
    err = optimizer_ai_init();
    if (err != OPTIMIZER_SUCCESS) {
        optimizer_gpu_cleanup();
        mtx_destroy(&g_state.mutex);
        return err;
    }

    // Set up supported formats
    g_state.num_supported_formats = 0;
    g_state.supported_formats[g_state.num_supported_formats++] = OPTIMIZER_FORMAT_JPEG;
    g_state.supported_formats[g_state.num_supported_formats++] = OPTIMIZER_FORMAT_PNG;
    g_state.supported_formats[g_state.num_supported_formats++] = OPTIMIZER_FORMAT_WEBP;
    g_state.supported_formats[g_state.num_supported_formats++] = OPTIMIZER_FORMAT_AVIF;

    // Create version string
    snprintf(g_state.version_string, sizeof(g_state.version_string), "%d.%d.%d",
             OPTIMIZER_VERSION_MAJOR, OPTIMIZER_VERSION_MINOR, OPTIMIZER_VERSION_PATCH);

    g_state.initialized = true;
    return OPTIMIZER_SUCCESS;
}

void optimizer_cleanup(void) {
    if (!g_state.initialized) {
        return;
    }

    optimizer_ai_cleanup();
    optimizer_gpu_cleanup();
    mtx_destroy(&g_state.mutex);
    g_state.initialized = false;
}

const char* optimizer_version_string(void) {
    return g_state.version_string;
}

const char* optimizer_error_string(optimizer_error_t error) {
    if (error >= 0 || -error > (int)(sizeof(error_messages) / sizeof(error_messages[0]))) {
        return "Unknown error";
    }
    return error_messages[-error];
}

optimizer_error_t optimizer_process_file(
    const char* input_path,
    const char* output_path,
    const optimizer_options_t* options,
    optimizer_progress_callback progress_cb,
    optimizer_preview_callback preview_cb,
    void* user_data
) {
    if (!g_state.initialized) {
        return OPTIMIZER_ERROR_INVALID_PARAM;
    }

    if (!input_path || !output_path || !options) {
        return OPTIMIZER_ERROR_INVALID_PARAM;
    }

    if (!is_format_supported(options->output_format)) {
        return OPTIMIZER_ERROR_UNSUPPORTED_FORMAT;
    }

    // Lock mutex for thread safety
    mtx_lock(&g_state.mutex);

    // Read input file
    FILE* fp = fopen(input_path, "rb");
    if (!fp) {
        mtx_unlock(&g_state.mutex);
        return OPTIMIZER_ERROR_FILE_IO;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate buffer
    uint8_t* buffer = malloc(file_size);
    if (!buffer) {
        fclose(fp);
        mtx_unlock(&g_state.mutex);
        return OPTIMIZER_ERROR_MEMORY;
    }

    // Read file
    if (fread(buffer, 1, file_size, fp) != file_size) {
        free(buffer);
        fclose(fp);
        mtx_unlock(&g_state.mutex);
        return OPTIMIZER_ERROR_FILE_IO;
    }

    fclose(fp);

    // Process buffer
    uint8_t* output_buffer = NULL;
    size_t output_size = 0;
    optimizer_error_t err = optimizer_process_buffer(
        buffer,
        file_size,
        &output_buffer,
        &output_size,
        options,
        progress_cb,
        preview_cb,
        user_data
    );

    free(buffer);

    if (err != OPTIMIZER_SUCCESS) {
        mtx_unlock(&g_state.mutex);
        return err;
    }

    // Write output file
    fp = fopen(output_path, "wb");
    if (!fp) {
        free(output_buffer);
        mtx_unlock(&g_state.mutex);
        return OPTIMIZER_ERROR_FILE_IO;
    }

    if (fwrite(output_buffer, 1, output_size, fp) != output_size) {
        free(output_buffer);
        fclose(fp);
        mtx_unlock(&g_state.mutex);
        return OPTIMIZER_ERROR_FILE_IO;
    }

    free(output_buffer);
    fclose(fp);
    mtx_unlock(&g_state.mutex);

    return OPTIMIZER_SUCCESS;
}

optimizer_error_t optimizer_process_buffer(
    const uint8_t* input_buffer,
    size_t input_size,
    uint8_t** output_buffer,
    size_t* output_size,
    const optimizer_options_t* options,
    optimizer_progress_callback progress_cb,
    optimizer_preview_callback preview_cb,
    void* user_data
) {
    if (!g_state.initialized) {
        return OPTIMIZER_ERROR_INVALID_PARAM;
    }

    if (!input_buffer || !output_buffer || !output_size || !options) {
        return OPTIMIZER_ERROR_INVALID_PARAM;
    }

    // Allocate GPU buffers
    optimizer_gpu_buffer_t input_gpu_buffer;
    optimizer_error_t err = optimizer_gpu_allocate(
        input_size,
        OPTIMIZER_GPU_MEMORY_DEVICE,
        &input_gpu_buffer
    );

    if (err != OPTIMIZER_SUCCESS) {
        return err;
    }

    // Copy input to GPU
    err = optimizer_gpu_memcpy_host_to_device(
        &input_gpu_buffer,
        input_buffer,
        input_size
    );

    if (err != OPTIMIZER_SUCCESS) {
        optimizer_gpu_free(&input_gpu_buffer);
        return err;
    }

    // Process on GPU
    optimizer_gpu_buffer_t output_gpu_buffer;
    err = optimizer_gpu_compress(
        &input_gpu_buffer,
        &output_gpu_buffer,
        output_size,
        options->max_width,
        options->max_height,
        options->output_format,
        options->quality
    );

    optimizer_gpu_free(&input_gpu_buffer);

    if (err != OPTIMIZER_SUCCESS) {
        return err;
    }

    // Allocate output buffer
    *output_buffer = malloc(*output_size);
    if (!*output_buffer) {
        optimizer_gpu_free(&output_gpu_buffer);
        return OPTIMIZER_ERROR_MEMORY;
    }

    // Copy result back to host
    err = optimizer_gpu_memcpy_device_to_host(
        *output_buffer,
        &output_gpu_buffer,
        *output_size
    );

    optimizer_gpu_free(&output_gpu_buffer);

    if (err != OPTIMIZER_SUCCESS) {
        free(*output_buffer);
        *output_buffer = NULL;
        *output_size = 0;
        return err;
    }

    return OPTIMIZER_SUCCESS;
}

optimizer_error_t optimizer_get_image_info(
    const char* path,
    optimizer_image_info_t* info
) {
    if (!g_state.initialized || !path || !info) {
        return OPTIMIZER_ERROR_INVALID_PARAM;
    }

    // Implementation will use image libraries to read metadata
    // This is a placeholder
    return OPTIMIZER_ERROR_INVALID_PARAM;
}

optimizer_error_t optimizer_get_supported_formats(
    optimizer_format_t** formats,
    size_t* count
) {
    if (!g_state.initialized || !formats || !count) {
        return OPTIMIZER_ERROR_INVALID_PARAM;
    }

    *formats = malloc(sizeof(optimizer_format_t) * g_state.num_supported_formats);
    if (!*formats) {
        return OPTIMIZER_ERROR_MEMORY;
    }

    memcpy(*formats, g_state.supported_formats,
           sizeof(optimizer_format_t) * g_state.num_supported_formats);
    *count = g_state.num_supported_formats;

    return OPTIMIZER_SUCCESS;
}

void optimizer_free_buffer(uint8_t* buffer) {
    free(buffer);
}

void optimizer_free_image_info(optimizer_image_info_t* info) {
    if (info) {
        free(info->metadata.exif_data);
        free(info->metadata.icc_profile);
        free(info->metadata.xmp_data);
    }
} 