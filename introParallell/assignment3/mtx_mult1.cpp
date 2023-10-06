#include <iostream>
#include <cstdlib>
#include <ctime>
#include <omp.h>

using namespace std;

// Function to initialize a matrix with random values
void initializeMatrix(double* matrix, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            matrix[i * size + j] = static_cast<double>(rand()) / RAND_MAX;
        }
    }
}

// Function to multiply two matrices
void matrixMultiply(double* A, double* B, double* C, int size) {
    #pragma omp parallel for
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            C[i * size + j] = 0.0;
            for (int k = 0; k < size; ++k) {
                C[i * size + j] += A[i * size + k] * B[k * size + j];
            }
        }
    }
}

void printMatrix(double* matrix, int size) {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            cout << matrix[i * size + j] << " ";
        }
        cout << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <num_threads> <matrix_size>" << endl;
        return 1;
    }

    int numThreads = atoi(argv[1]);
    int matrixSize = atoi(argv[2]);

    // Set the number of threads using OpenMP environment variable
    omp_set_num_threads(numThreads);

    double* A = new double[matrixSize * matrixSize];
    double* B = new double[matrixSize * matrixSize];
    double* C = new double[matrixSize * matrixSize];

    // Initialize matrices A and B with random values
    srand(static_cast<unsigned>(time(nullptr)));
    initializeMatrix(A, matrixSize);
    initializeMatrix(B, matrixSize);

    // Measure execution time
    double startTime = omp_get_wtime();

    // Perform matrix multiplication
    matrixMultiply(A, B, C, matrixSize);

    double endTime = omp_get_wtime();
    double executionTime = endTime - startTime;

    // Print the result and execution time
    cout << "Matrix multiplication result for matrix size " << matrixSize << ":" << endl;

      // Print the result matrix if the matrix size is reasonable
    if (matrixSize <= 10) { // Adjust the threshold as needed
        cout << "Matrix C (Result):" << endl;
        printMatrix(C, matrixSize);
    } else {
        cout << "Matrix multiplication result for matrix size " << matrixSize << " is too large to print." << endl;
    }

    cout << "Execution time with " << numThreads << " threads: " << executionTime << " seconds" << endl;



    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
