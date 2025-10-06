#include "kernel/types.h"
#include "user/user.h"

// Matrix multiplication function
void matrix_multiply(int *A, int *B, int *C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i * n + j] = 0;
            for (int k = 0; k < n; k++) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
}

void init_matrix(int *matrix, int n, int seed) {
    for (int i = 0; i < n * n; i++) {
        matrix[i] = (i + seed) % 100;
    }
}

void print_matrix(int *matrix, int n, const char *name) {
    if (n > 4) return;
    
    printf("%s:\n", name);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", matrix[i * n + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    printf("Matrix Multiplication Benchmark\n");
    
    // Test different matrix sizes
    int sizes[] = {128, 256, 512};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        printf("Testing matrix size: %d x %d\n", n, n);
        
        // Allocate memory
        int size_bytes = n * n * sizeof(int);
        int *A = malloc(size_bytes);
        int *B = malloc(size_bytes);
        int *C = malloc(size_bytes);
        
        if (!A || !B || !C) {
            continue;
        }
        
        init_matrix(A, n, 1);
        init_matrix(B, n, 2);
        
        // Start timing
        uint64 start_time = rtime();
        uint64 start_cycles = rcycle();
        uint64 start_instret = rinstret();
        
        matrix_multiply(A, B, C, n);
        
        // End timing
        uint64 end_time = rtime();
        uint64 end_cycles = rcycle();
        uint64 end_instret = rinstret();
        
        uint64 exec_time = end_time - start_time;
        uint64 cycles = end_cycles - start_cycles;
        uint64 instructions = end_instret - start_instret;
        
        printf("  Execution time: %lu units\n", exec_time);
        printf("  CPU cycles: %lu\n", cycles);
        printf("  Instructions: %lu\n", instructions);
        
        printf("\n");
        
        free(A);
        free(B);
        free(C);
        
        // Small delay
        sleep(1);
    }
    
    printf("Benchmark completed!\n");
    exit(0);
}