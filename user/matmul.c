#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define SIZE 10

int A[SIZE][SIZE] = {
    {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
    {11, 12, 13, 14, 15, 16, 17, 18, 19, 20},
    {21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
    {31, 32, 33, 34, 35, 36, 37, 38, 39, 40},
    {41, 42, 43, 44, 45, 46, 47, 48, 49, 50},
    {51, 52, 53, 54, 55, 56, 57, 58, 59, 60},
    {61, 62, 63, 64, 65, 66, 67, 68, 69, 70},
    {71, 72, 73, 74, 75, 76, 77, 78, 79, 80},
    {81, 82, 83, 84, 85, 86, 87, 88, 89, 90},
    {91, 92, 93, 94, 95, 96, 97, 98, 99, 100}};

int B[SIZE][SIZE] = {
    {10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
    {20, 19, 18, 17, 16, 15, 14, 13, 12, 11},
    {30, 29, 28, 27, 26, 25, 24, 23, 22, 21},
    {40, 39, 38, 37, 36, 35, 34, 33, 32, 31},
    {50, 49, 48, 47, 46, 45, 44, 43, 42, 41},
    {60, 59, 58, 57, 56, 55, 54, 53, 52, 51},
    {70, 69, 68, 67, 66, 65, 64, 63, 62, 61},
    {80, 79, 78, 77, 76, 75, 74, 73, 72, 71},
    {90, 89, 88, 87, 86, 85, 84, 83, 82, 81},
    {100, 99, 98, 97, 96, 95, 94, 93, 92, 91}
};

int main(int argc, char *argv[])
{
    int C[SIZE][SIZE];
    int fd[2];
    pipe(fd);

    for (int i = 0; i < SIZE; i++) {
        if (fork() == 0) {  // Child process
            close(fd[0]);  // Close read end
            int row[SIZE];
            for (int j = 0; j < SIZE; j++) {
                row[j] = 0;
                for (int k = 0; k < SIZE; k++) {
                    row[j] += A[i][k] * B[k][j];
                }
            }
            write(fd[1], &i, sizeof(int)); // Send row index
            write(fd[1], row, sizeof(row)); // Send computed row
            close(fd[1]);
            exit(1);
        }
    }

    // Parent process
    close(fd[1]);  // Close write end
    for (int i = 0; i < SIZE; i++) {
        int index;
        read(fd[0], &index, sizeof(int)); // Read row index
        read(fd[0], C[index], sizeof(C[index])); // Read row data
    }
    close(fd[0]);

    // Wait for all children
    for (int i = 0; i < SIZE; i++) wait(0);

    // Print result matrix C
    printf("Result Matrix C:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", C[i][j]);
        }
        printf("\n");
    }
    exit(1);
}