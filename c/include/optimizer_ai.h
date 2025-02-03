#ifndef OPTIMIZER_AI_H
#define OPTIMIZER_AI_H

#include "optimizer.h"

#ifdef __cplusplus
extern "C" {
#endif

// AI Model types
typedef enum {
    OPTIMIZER_AI_MODEL_SUPER_RESOLUTION,
    OPTIMIZER_AI_MODEL_DENOISE,
    OPTIMIZER_AI_MODEL_FACE_ENHANCE,
    OPTIMIZER_AI_MODEL_OBJECT_DETECTION,
    OPTIMIZER_AI_MODEL_SEGMENTATION,
    OPTIMIZER_AI_MODEL_STYLE_TRANSFER,
    OPTIMIZER_AI_MODEL_COLOR_ENHANCE
} optimizer_ai_model_t;

// AI Model information
typedef struct {
    char* name;
    char* version;
    size_t input_width;
    size_t input_height;
    size_t channels;
    float* mean;
    float* std;
    char* backend;
    size_t memory_required;
} optimizer_ai_model_info_t;

// Object detection result
typedef struct {
    char* class_name;
    float confidence;
    float x;
    float y;
    float width;
    float height;
} optimizer_ai_object_t;

// Segmentation mask
typedef struct {
    uint8_t* mask;
    size_t width;
    size_t height;
    char* class_name;
    float confidence;
} optimizer_ai_mask_t;

// Face detection result
typedef struct {
    float x;
    float y;
    float width;
    float height;
    float* landmarks;
    size_t num_landmarks;
    float confidence;
    char* attributes;
} optimizer_ai_face_t;

// Style transfer options
typedef struct {
    char* style_name;
    float strength;
    bool preserve_color;
    bool high_resolution;
} optimizer_ai_style_options_t;

// Enhancement options
typedef struct {
    float denoise_strength;
    float sharpen_strength;
    float color_enhance_strength;
    float exposure_adjust;
    float contrast_adjust;
    bool fix_red_eye;
    bool fix_skin_tone;
} optimizer_ai_enhance_options_t;

// AI initialization and cleanup
optimizer_error_t optimizer_ai_init(void);
void optimizer_ai_cleanup(void);

// Model management
optimizer_error_t optimizer_ai_load_model(
    optimizer_ai_model_t model_type,
    const char* model_path
);
optimizer_error_t optimizer_ai_unload_model(optimizer_ai_model_t model_type);
optimizer_error_t optimizer_ai_get_model_info(
    optimizer_ai_model_t model_type,
    optimizer_ai_model_info_t* info
);

// Super resolution
optimizer_error_t optimizer_ai_super_resolution(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    uint32_t scale_factor,
    optimizer_progress_callback progress_cb,
    void* user_data
);

// Denoising
optimizer_error_t optimizer_ai_denoise(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    float strength,
    optimizer_progress_callback progress_cb,
    void* user_data
);

// Object detection
optimizer_error_t optimizer_ai_detect_objects(
    const optimizer_gpu_buffer_t* input,
    optimizer_ai_object_t** objects,
    size_t* num_objects,
    float confidence_threshold,
    optimizer_progress_callback progress_cb,
    void* user_data
);

// Segmentation
optimizer_error_t optimizer_ai_segment_image(
    const optimizer_gpu_buffer_t* input,
    optimizer_ai_mask_t** masks,
    size_t* num_masks,
    float confidence_threshold,
    optimizer_progress_callback progress_cb,
    void* user_data
);

// Face detection and enhancement
optimizer_error_t optimizer_ai_detect_faces(
    const optimizer_gpu_buffer_t* input,
    optimizer_ai_face_t** faces,
    size_t* num_faces,
    float confidence_threshold,
    optimizer_progress_callback progress_cb,
    void* user_data
);

optimizer_error_t optimizer_ai_enhance_faces(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    const optimizer_ai_face_t* faces,
    size_t num_faces,
    const optimizer_ai_enhance_options_t* options,
    optimizer_progress_callback progress_cb,
    void* user_data
);

// Style transfer
optimizer_error_t optimizer_ai_apply_style(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    const optimizer_ai_style_options_t* options,
    optimizer_progress_callback progress_cb,
    void* user_data
);

// Color enhancement
optimizer_error_t optimizer_ai_enhance_color(
    const optimizer_gpu_buffer_t* input,
    optimizer_gpu_buffer_t* output,
    const optimizer_ai_enhance_options_t* options,
    optimizer_progress_callback progress_cb,
    void* user_data
);

// Memory management
void optimizer_ai_free_objects(optimizer_ai_object_t* objects, size_t count);
void optimizer_ai_free_masks(optimizer_ai_mask_t* masks, size_t count);
void optimizer_ai_free_faces(optimizer_ai_face_t* faces, size_t count);
void optimizer_ai_free_model_info(optimizer_ai_model_info_t* info);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZER_AI_H 