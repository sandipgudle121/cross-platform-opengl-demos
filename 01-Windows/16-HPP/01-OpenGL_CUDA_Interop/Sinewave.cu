#include "Sinewave.cu.h"

// CUDA kernel for sinwave
__global__ void sineWaveKernel(float4 *pos, int width, int height, float time)
{
    // Code
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    float u = (float)x / (float)width;
    float v = (float)y / (float)height;
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    float frequency = 4.0f;
    float w = sinf(u * frequency + time) * cosf(v * frequency + time) * 0.5f;

    pos[y * width + x] = make_float4(u, w, v, 1.0f);
}

// User difined to call CUDA kernel
void launchCUDAKernel(float4 *pos, int width, int height, float time)
{
    dim3 block(8, 8, 1);
    dim3 grid(width / block.x, height / block.y, 1);

    sineWaveKernel <<< grid, block >>> (pos, width, height, time);
}
