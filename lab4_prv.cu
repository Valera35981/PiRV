#include <iostream>
#include <cuda_runtime.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>

using namespace std;

void powerArrayCPU(const float* A, float* B, int N, float p) {
    for (int i = 0; i < N; i++) {
        B[i] = powf(A[i], p);
    }
}

__global__ void powerArrayCUDA(const float* A, float* B, int N, float p) {
    int idx = threadIdx.x + blockIdx.x * blockDim.x;
    if (idx < N) {
        B[idx] = powf(A[idx], p);
    }
}

void rotateImageCPU(const unsigned char* input, unsigned char* output, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int srcX = height - x - 1;
            int srcY = y;
            output[y * width + x] = input[srcY * width + srcX];
        }
    }
}

__global__ void rotateImageCUDA(const unsigned char* input, unsigned char* output, int width, int height) {
    int x = threadIdx.x + blockIdx.x * blockDim.x;
    int y = threadIdx.y + blockIdx.y * blockDim.y;

    if (x < width && y < height) {
        int srcX = height - x - 1;
        int srcY = y;
        output[y * width + x] = input[srcY * width + srcX];
    }
}

bool compareResults(const unsigned char* cpu, const unsigned char* gpu, int size) {
    for (int i = 0; i < size; i++) {
        if (cpu[i] != gpu[i]) {
            cout << "Mismatch at index " << i << ": CPU=" << (int)cpu[i] << ", GPU=" << (int)gpu[i] << endl;
            return false;
        }
    }
    return true;
}

int main() {
    cout << "========== TASK 1: Power Array ==========" << endl;

    const int N = 500000;
    const float p = 0.5f;

    float* h_A = new float[N];
    float* h_B_CPU = new float[N];
    float* h_B_GPU = new float[N];

    srand((unsigned int)time(nullptr));
    for (int i = 0; i < N; i++) {
        h_A[i] = (float)(rand() % 1000) / 100.0f;
    }

    clock_t startCPU = clock();
    powerArrayCPU(h_A, h_B_CPU, N, p);
    clock_t endCPU = clock();
    double timeCPU = double(endCPU - startCPU) / CLOCKS_PER_SEC * 1000.0;

    float* d_A, * d_B;
    size_t bytes = N * sizeof(float);

    cudaMalloc(&d_A, bytes);
    cudaMalloc(&d_B, bytes);
    cudaMemcpy(d_A, h_A, bytes, cudaMemcpyHostToDevice);

    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);
    powerArrayCUDA << <blocksPerGrid, threadsPerBlock >> > (d_A, d_B, N, p);
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float timeGPUms = 0;
    cudaEventElapsedTime(&timeGPUms, start, stop);

    cudaMemcpy(h_B_GPU, d_B, bytes, cudaMemcpyDeviceToHost);

    cout << "\nFirst 10 results:" << endl;
    bool task1Correct = true;
    for (int i = 0; i < 10; i++) {
        cout << "A[" << i << "]=" << fixed << setprecision(4) << h_A[i]
            << " sqrt=" << h_B_GPU[i] << " (CPU=" << h_B_CPU[i] << ")" << endl;
        if (fabs(h_B_CPU[i] - h_B_GPU[i]) > 0.0001f) task1Correct = false;
    }

    cout << "\nCPU time: " << timeCPU << " ms" << endl;
    cout << "GPU time: " << timeGPUms << " ms" << endl;
    cout << "Speedup: " << (timeCPU / timeGPUms) << "x" << endl;
    cout << "Correctness: " << (task1Correct ? "OK" : "ERROR") << endl;

    cout << "\n========== TASK 2: Rotate Image 90 ==========" << endl;

    const int WIDTH = 512;
    const int HEIGHT = 512;
    const int IMG_SIZE = WIDTH * HEIGHT;

    unsigned char* h_img = new unsigned char[IMG_SIZE];
    unsigned char* h_rot_cpu = new unsigned char[IMG_SIZE];
    unsigned char* h_rot_gpu = new unsigned char[IMG_SIZE];

    for (int i = 0; i < IMG_SIZE; i++) {
        h_img[i] = rand() % 256;
    }

    startCPU = clock();
    rotateImageCPU(h_img, h_rot_cpu, WIDTH, HEIGHT);
    endCPU = clock();
    timeCPU = double(endCPU - startCPU) / CLOCKS_PER_SEC * 1000.0;

    unsigned char* d_input, * d_output;
    size_t imgBytes = IMG_SIZE * sizeof(unsigned char);

    cudaMalloc(&d_input, imgBytes);
    cudaMalloc(&d_output, imgBytes);
    cudaMemcpy(d_input, h_img, imgBytes, cudaMemcpyHostToDevice);

    dim3 threadsPerBlock2D(16, 16);
    dim3 blocksPerGrid2D(
        (WIDTH + threadsPerBlock2D.x - 1) / threadsPerBlock2D.x,
        (HEIGHT + threadsPerBlock2D.y - 1) / threadsPerBlock2D.y
    );

    cudaEventRecord(start);
    rotateImageCUDA << <blocksPerGrid2D, threadsPerBlock2D >> > (d_input, d_output, WIDTH, HEIGHT);
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    cudaEventElapsedTime(&timeGPUms, start, stop);
    cudaMemcpy(h_rot_gpu, d_output, imgBytes, cudaMemcpyDeviceToHost);

    bool task2Correct = compareResults(h_rot_cpu, h_rot_gpu, IMG_SIZE);

    cout << "\nFirst 5x5 of rotated image (CPU vs GPU):" << endl;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            cout << setw(4) << (int)h_rot_cpu[y * WIDTH + x];
        }
        cout << "   ";
        for (int x = 0; x < 5; x++) {
            cout << setw(4) << (int)h_rot_gpu[y * WIDTH + x];
        }
        cout << endl;
    }

    cout << "\nCPU time: " << timeCPU << " ms" << endl;
    cout << "GPU time: " << timeGPUms << " ms" << endl;
    cout << "Speedup: " << (timeCPU / timeGPUms) << "x" << endl;
    cout << "Correctness: " << (task2Correct ? "OK" : "ERROR") << endl;

    delete[] h_A; delete[] h_B_CPU; delete[] h_B_GPU;
    delete[] h_img; delete[] h_rot_cpu; delete[] h_rot_gpu;
    cudaFree(d_A); cudaFree(d_B);
    cudaFree(d_input); cudaFree(d_output);
    cudaEventDestroy(start); cudaEventDestroy(stop);

    cout << "\n=== PROGRAM FINISHED SUCCESSFULLY ===" << endl;

    return 0;
}