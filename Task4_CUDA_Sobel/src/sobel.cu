#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <cuda_runtime.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#define MAX_PATH 512

void checkCuda(cudaError_t result, const char *message) {
    if (result != cudaSuccess) {
        printf("CUDA Error: %s : %s\n", message, cudaGetErrorString(result));
        exit(1);
    }
}

int isPngFile(const char *fileName) {
    int length = strlen(fileName);

    if (length < 4) {
        return 0;
    }

    const char *extension = fileName + length - 4;

    if (strcmp(extension, ".png") == 0 || strcmp(extension, ".PNG") == 0) {
        return 1;
    }

    return 0;
}

// Sobel kernel with zero padding
__global__ void sobelKernel(unsigned char *input, unsigned char *output, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) {
        return;
    }

    int Gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int Gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    int sumX = 0;
    int sumY = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int pixelX = x + j;
            int pixelY = y + i;

            int pixel = 0;

            // Zero padding
            if (pixelX >= 0 && pixelX < width && pixelY >= 0 && pixelY < height) {
                pixel = input[pixelY * width + pixelX];
            }

            sumX += pixel * Gx[i + 1][j + 1];
            sumY += pixel * Gy[i + 1][j + 1];
        }
    }

    float magnitudeValue = sqrtf((float)(sumX * sumX + sumY * sumY));

    // Brightness factor for clearer output
    int magnitude = (int)(magnitudeValue * 2.5f);

    if (magnitude > 255) {
        magnitude = 255;
    }

    if (magnitude < 0) {
        magnitude = 0;
    }

    output[y * width + x] = (unsigned char)magnitude;
}

void processImage(const char *inputPath, const char *outputPath) {
    int width, height, channels;

    unsigned char *inputImage = stbi_load(inputPath, &width, &height, &channels, 1);

    if (!inputImage) {
        printf("Image load failed: %s\n", inputPath);
        return;
    }

    int size = width * height * sizeof(unsigned char);

    unsigned char *outputImage = (unsigned char *)malloc(size);

    if (outputImage == NULL) {
        printf("Output memory allocation failed\n");
        stbi_image_free(inputImage);
        return;
    }

    unsigned char *d_input = NULL;
    unsigned char *d_output = NULL;

    checkCuda(cudaMalloc((void **)&d_input, size), "Allocating GPU input memory");
    checkCuda(cudaMalloc((void **)&d_output, size), "Allocating GPU output memory");

    checkCuda(cudaMemcpy(d_input, inputImage, size, cudaMemcpyHostToDevice),
              "Copying image from CPU to GPU");

    dim3 threads(16, 16);
    dim3 blocks((width + threads.x - 1) / threads.x,
                (height + threads.y - 1) / threads.y);

    sobelKernel<<<blocks, threads>>>(d_input, d_output, width, height);

    checkCuda(cudaGetLastError(), "Launching Sobel kernel");
    checkCuda(cudaDeviceSynchronize(), "Synchronizing GPU");

    checkCuda(cudaMemcpy(outputImage, d_output, size, cudaMemcpyDeviceToHost),
              "Copying output image from GPU to CPU");

    stbi_write_png(outputPath, width, height, 1, outputImage, width);

    stbi_image_free(inputImage);
    free(outputImage);
    cudaFree(d_input);
    cudaFree(d_output);

    printf("Processed: %s\n", inputPath);
    printf("Saved    : %s\n", outputPath);
}

int main() {
    const char *inputFolder = "6CS005_HPC_Assignment/Task4_CUDA_Sobel/images/input";
    const char *outputFolder = "6CS005_HPC_Assignment/Task4_CUDA_Sobel/images/output";

    DIR *folder = opendir(inputFolder);

    if (folder == NULL) {
        printf("Cannot open input folder\n");
        return 1;
    }

    struct dirent *entry;
    int imageCount = 0;

    while ((entry = readdir(folder)) != NULL) {
        if (isPngFile(entry->d_name)) {
            char inputPath[MAX_PATH];
            char outputPath[MAX_PATH];

            snprintf(inputPath, sizeof(inputPath), "%s/%s", inputFolder, entry->d_name);
            snprintf(outputPath, sizeof(outputPath), "%s/sobel_%s", outputFolder, entry->d_name);

            processImage(inputPath, outputPath);
            imageCount++;
        }
    }

    closedir(folder);

    printf("Task 4 completed. Total PNG images processed: %d\n", imageCount);

    return 0;
}
