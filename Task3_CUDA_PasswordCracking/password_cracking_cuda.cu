#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda_runtime.h>

#define MAX_PASSWORD_LENGTH 64
#define MAX_PASSWORD_COUNT 10000

void check_cuda_error(cudaError_t error, const char *message) {
    if (error != cudaSuccess) {
        printf("CUDA Error: %s : %s\n", message, cudaGetErrorString(error));
        exit(1);
    }
}

__device__ void device_decrypt_password(char *encrypted_password, char *decrypted_password) {
    int i = 0;

    while (i < MAX_PASSWORD_LENGTH && encrypted_password[i] != '\0') {
        decrypted_password[i] = encrypted_password[i] - 2;
        i++;
    }

    decrypted_password[i] = '\0';
}

__global__ void password_cracking_kernel(char *device_encrypted_passwords,
                                         char *device_decrypted_passwords,
                                         int password_count) {
    int password_index = blockIdx.x * blockDim.x + threadIdx.x;

    if (password_index < password_count) {
        char *encrypted_password = device_encrypted_passwords + password_index * MAX_PASSWORD_LENGTH;
        char *decrypted_password = device_decrypted_passwords + password_index * MAX_PASSWORD_LENGTH;

        device_decrypt_password(encrypted_password, decrypted_password);
    }
}

int read_encrypted_passwords(const char *input_file_name,
                             char encrypted_passwords[][MAX_PASSWORD_LENGTH]) {
    FILE *input_file = fopen(input_file_name, "r");

    if (input_file == NULL) {
        printf("Error: Cannot open input file.\n");
        return -1;
    }

    int password_count = 0;

    while (password_count < MAX_PASSWORD_COUNT &&
           fscanf(input_file, "%63s", encrypted_passwords[password_count]) == 1) {
        password_count++;
    }

    fclose(input_file);
    return password_count;
}

void write_decrypted_passwords(const char *output_file_name,
                               char decrypted_passwords[][MAX_PASSWORD_LENGTH],
                               int password_count) {
    FILE *output_file = fopen(output_file_name, "w");

    if (output_file == NULL) {
        printf("Error: Cannot create output file.\n");
        return;
    }

    fprintf(output_file, "Password Cracking using CUDA\n");
    fprintf(output_file, "Total Passwords: %d\n", password_count);
    fprintf(output_file, "--------------------------------\n");
    fprintf(output_file, "%-20s %s\n", "Password No", "Decrypted Password");
    fprintf(output_file, "--------------------------------\n");

    for (int i = 0; i < password_count; i++) {
        fprintf(output_file, "%-20d %s\n", i + 1, decrypted_passwords[i]);
    }

    fclose(output_file);
}

void create_output_file_path(const char *input_file_path,
                             char *output_file_path,
                             int path_size) {
    const char *last_slash = strrchr(input_file_path, '/');

    if (last_slash == NULL) {
        snprintf(output_file_path, path_size, "decrypted.txt");
    } else {
        int folder_length = last_slash - input_file_path + 1;
        snprintf(output_file_path, path_size, "%.*sdecrypted.txt", folder_length, input_file_path);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <encrypted_password_file>\n", argv[0]);
        return 1;
    }

    const char *input_file_name = argv[1];

    char encrypted_passwords[MAX_PASSWORD_COUNT][MAX_PASSWORD_LENGTH];
    char decrypted_passwords[MAX_PASSWORD_COUNT][MAX_PASSWORD_LENGTH];

    int password_count = read_encrypted_passwords(input_file_name, encrypted_passwords);

    if (password_count <= 0) {
        printf("Error: No encrypted passwords found.\n");
        return 1;
    }

    size_t memory_size = password_count * MAX_PASSWORD_LENGTH * sizeof(char);

    char *device_encrypted_passwords = NULL;
    char *device_decrypted_passwords = NULL;

    check_cuda_error(
        cudaMalloc((void **)&device_encrypted_passwords, memory_size),
        "Allocating memory for encrypted passwords"
    );

    check_cuda_error(
        cudaMalloc((void **)&device_decrypted_passwords, memory_size),
        "Allocating memory for decrypted passwords"
    );

    check_cuda_error(
        cudaMemcpy(device_encrypted_passwords,
                   encrypted_passwords,
                   memory_size,
                   cudaMemcpyHostToDevice),
        "Copying encrypted passwords from CPU to GPU"
    );

    int threads_per_block = 256;

    if (password_count < threads_per_block) {
        threads_per_block = password_count;
    }

    int blocks_per_grid = (password_count + threads_per_block - 1) / threads_per_block;

    password_cracking_kernel<<<blocks_per_grid, threads_per_block>>>(
        device_encrypted_passwords,
        device_decrypted_passwords,
        password_count
    );

    check_cuda_error(cudaGetLastError(), "Launching CUDA kernel");
    check_cuda_error(cudaDeviceSynchronize(), "Synchronizing CUDA device");

    check_cuda_error(
        cudaMemcpy(decrypted_passwords,
                   device_decrypted_passwords,
                   memory_size,
                   cudaMemcpyDeviceToHost),
        "Copying decrypted passwords from GPU to CPU"
    );

    char output_file_path[300];
    create_output_file_path(input_file_name, output_file_path, sizeof(output_file_path));

    write_decrypted_passwords(output_file_path, decrypted_passwords, password_count);

    cudaFree(device_encrypted_passwords);
    cudaFree(device_decrypted_passwords);

    printf("Task 3 completed successfully.\n");
    printf("Passwords processed: %d\n", password_count);
    printf("Blocks used: %d\n", blocks_per_grid);
    printf("Threads per block: %d\n", threads_per_block);
    printf("Output written to: %s\n", output_file_path);

    return 0;
}
