#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <npp.h>
#include "optimizer_gpu.h"

// Texture objects for efficient image access
texture<float4, cudaTextureType2D, cudaReadModeElementType> texInput;
texture<float4, cudaTextureType2D, cudaReadModeElementType> texWatermark;

// Constants
#define BLOCK_SIZE 16
#define MAX_THREADS_PER_BLOCK 1024

// Helper functions
__device__ float4 apply_color_correction(float4 color, float contrast, float brightness, float saturation) {
    // Convert to HSV
    float minVal = fminf(fminf(color.x, color.y), color.z);
    float maxVal = fmaxf(fmaxf(color.x, color.y), color.z);
    float delta = maxVal - minVal;
    
    float h = 0;
    float s = maxVal == 0 ? 0 : delta / maxVal;
    float v = maxVal;
    
    if (delta > 0) {
        if (maxVal == color.x) {
            h = (color.y - color.z) / delta + (color.y < color.z ? 6.0f : 0.0f);
        } else if (maxVal == color.y) {
            h = (color.z - color.x) / delta + 2.0f;
        } else {
            h = (color.x - color.y) / delta + 4.0f;
        }
        h /= 6.0f;
    }
    
    // Apply adjustments
    s = fminf(1.0f, s * saturation);
    v = fminf(1.0f, v * brightness);
    v = (v - 0.5f) * contrast + 0.5f;
    v = fmaxf(0.0f, fminf(1.0f, v));
    
    // Convert back to RGB
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h * 6.0f, 2.0f) - 1));
    float m = v - c;
    
    float4 result;
    if (h < 1.0f/6.0f) {
        result = make_float4(c + m, x + m, m, color.w);
    } else if (h < 2.0f/6.0f) {
        result = make_float4(x + m, c + m, m, color.w);
    } else if (h < 3.0f/6.0f) {
        result = make_float4(m, c + m, x + m, color.w);
    } else if (h < 4.0f/6.0f) {
        result = make_float4(m, x + m, c + m, color.w);
    } else if (h < 5.0f/6.0f) {
        result = make_float4(x + m, m, c + m, color.w);
    } else {
        result = make_float4(c + m, m, x + m, color.w);
    }
    
    return result;
}

__device__ float4 apply_sharpening(float4 center, float4 top, float4 bottom, float4 left, float4 right, float strength) {
    float4 laplacian = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
    laplacian.x = 4 * center.x - top.x - bottom.x - left.x - right.x;
    laplacian.y = 4 * center.y - top.y - bottom.y - left.y - right.y;
    laplacian.z = 4 * center.z - top.z - bottom.z - left.z - right.z;
    
    float4 result;
    result.x = center.x + strength * laplacian.x;
    result.y = center.y + strength * laplacian.y;
    result.z = center.z + strength * laplacian.z;
    result.w = center.w;
    
    return result;
}

// Kernel implementations
extern "C" {

__global__ void resizeKernel(
    float4* output,
    int outputWidth,
    int outputHeight,
    float scaleX,
    float scaleY
) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= outputWidth || y >= outputHeight)
        return;
        
    float u = (x + 0.5f) * scaleX;
    float v = (y + 0.5f) * scaleY;
    
    float4 pixel = tex2D(texInput, u, v);
    output[y * outputWidth + x] = pixel;
}

__global__ void colorCorrectKernel(
    float4* output,
    int width,
    int height,
    float contrast,
    float brightness,
    float saturation
) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= width || y >= height)
        return;
        
    float4 pixel = tex2D(texInput, x + 0.5f, y + 0.5f);
    pixel = apply_color_correction(pixel, contrast, brightness, saturation);
    output[y * width + x] = pixel;
}

__global__ void sharpenKernel(
    float4* output,
    int width,
    int height,
    float strength
) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= width || y >= height)
        return;
        
    float4 center = tex2D(texInput, x + 0.5f, y + 0.5f);
    float4 top = tex2D(texInput, x + 0.5f, y - 0.5f);
    float4 bottom = tex2D(texInput, x + 0.5f, y + 1.5f);
    float4 left = tex2D(texInput, x - 0.5f, y + 0.5f);
    float4 right = tex2D(texInput, x + 1.5f, y + 0.5f);
    
    output[y * width + x] = apply_sharpening(center, top, bottom, left, right, strength);
}

__global__ void watermarkKernel(
    float4* output,
    int width,
    int height,
    int watermarkWidth,
    int watermarkHeight,
    int offsetX,
    int offsetY,
    float opacity
) {
    const int x = blockIdx.x * blockDim.x + threadIdx.x;
    const int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x >= width || y >= height)
        return;
        
    float4 pixel = tex2D(texInput, x + 0.5f, y + 0.5f);
    
    // Check if pixel is in watermark area
    if (x >= offsetX && x < offsetX + watermarkWidth &&
        y >= offsetY && y < offsetY + watermarkHeight) {
        float watermarkU = (float)(x - offsetX) / watermarkWidth;
        float watermarkV = (float)(y - offsetY) / watermarkHeight;
        float4 watermark = tex2D(texWatermark, watermarkU, watermarkV);
        
        // Alpha blending
        float alpha = watermark.w * opacity;
        pixel.x = (1 - alpha) * pixel.x + alpha * watermark.x;
        pixel.y = (1 - alpha) * pixel.y + alpha * watermark.y;
        pixel.z = (1 - alpha) * pixel.z + alpha * watermark.z;
    }
    
    output[y * width + x] = pixel;
}

} // extern "C"

// Kernel launch helpers
cudaError_t launchResizeKernel(
    float4* output,
    int outputWidth,
    int outputHeight,
    float scaleX,
    float scaleY
) {
    dim3 block(BLOCK_SIZE, BLOCK_SIZE);
    dim3 grid(
        (outputWidth + block.x - 1) / block.x,
        (outputHeight + block.y - 1) / block.y
    );
    
    resizeKernel<<<grid, block>>>(
        output,
        outputWidth,
        outputHeight,
        scaleX,
        scaleY
    );
    
    return cudaGetLastError();
}

cudaError_t launchColorCorrectKernel(
    float4* output,
    int width,
    int height,
    float contrast,
    float brightness,
    float saturation
) {
    dim3 block(BLOCK_SIZE, BLOCK_SIZE);
    dim3 grid(
        (width + block.x - 1) / block.x,
        (height + block.y - 1) / block.y
    );
    
    colorCorrectKernel<<<grid, block>>>(
        output,
        width,
        height,
        contrast,
        brightness,
        saturation
    );
    
    return cudaGetLastError();
}

cudaError_t launchSharpenKernel(
    float4* output,
    int width,
    int height,
    float strength
) {
    dim3 block(BLOCK_SIZE, BLOCK_SIZE);
    dim3 grid(
        (width + block.x - 1) / block.x,
        (height + block.y - 1) / block.y
    );
    
    sharpenKernel<<<grid, block>>>(
        output,
        width,
        height,
        strength
    );
    
    return cudaGetLastError();
}

cudaError_t launchWatermarkKernel(
    float4* output,
    int width,
    int height,
    int watermarkWidth,
    int watermarkHeight,
    int offsetX,
    int offsetY,
    float opacity
) {
    dim3 block(BLOCK_SIZE, BLOCK_SIZE);
    dim3 grid(
        (width + block.x - 1) / block.x,
        (height + block.y - 1) / block.y
    );
    
    watermarkKernel<<<grid, block>>>(
        output,
        width,
        height,
        watermarkWidth,
        watermarkHeight,
        offsetX,
        offsetY,
        opacity
    );
    
    return cudaGetLastError();
} 