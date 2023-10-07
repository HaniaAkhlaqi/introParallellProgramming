#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>
#include <omp.h> // Include OpenMP header

#define FINALIZE "\
ffmpeg -y -start_number 0 -i out%d.pgm output.gif\n\
rm *pgm\n\
"

// Function to allocate a 2D integer array of size N x N and initialize it with zeros.
static int** allocate_array(int N) {
    int** array;
    int i, j;
    array = new int*[N];
    for (i = 0; i < N; i++)
        array[i] = new int[N];
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            array[i][j] = 0;
    return array; // return pointer to the array
}

// Function to deallocate memory previously allocated for a 2D integer array.
static void free_array(int** array, int N) {
    for (int i = 0; i < N; i++)
        delete[] array[i];
    delete[] array;
}

// Function to randomly initialize two 2D arrays array1 and array2 with a pattern of ones in approximately 10% of the cells.
static void init_random(int** array1, int** array2, int N) {
    int i, pos;
// Parallel Initialization
#pragma omp parallel for private(i, pos) shared(array1, array2)
    for (i = 0; i < (N * N) / 10; i++) {
        pos = rand() % ((N - 2) * (N - 2));
        #pragma omp critical
        array1[pos % (N - 2) + 1][pos / (N - 2) + 1] = 1;
        array2[pos % (N - 2) + 1][pos / (N - 2) + 1] = 1;
    }
}

// Function to print the grid to a PGM image at a given time step.
static void print_to_pgm(int** array, int N, int t) {
    int i, j;
    char s[30];
    sprintf(s, "out%d.pgm", t);
    std::ofstream f(s, std::ios::binary);
    f << "P5\n" << N << " " << N << " 1\n";
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            if (array[i][j] == 1)
                f.put(1);
            else
                f.put(0);
}

int main(int argc, char* argv[]) {
    int N;             // array dimensions
    int T;             // time steps
    int** current, ** previous; // arrays - one for the current timestep, one for the previous timestep
    int** swap;        // array pointer
    int t, i, j, nbrs; // helper variables

    double time;       // variable for timing
    double ts, tf;

    int num_threads = 1;  // Default to one thread

    /* Read input arguments */
    if (argc != 4) {
        std::cerr << "Usage: ./exec ArraySize TimeSteps s NumThreads\n";
        exit(-1);
    }
    else {
        N = atoi(argv[1]);
        T = atoi(argv[2]);
        num_threads = atoi(argv[3]);  // Read the number of threads from command line
    }

    omp_set_num_threads(num_threads);  // Set the number of threads

    /* Allocate and initialize matrices */
    current = allocate_array(N);         // allocate array for the current time step
    previous = allocate_array(N);        // allocate array for the previous time step
    swap = allocate_array(N);            // allocate memory for the swap array


    std::srand(std::time(nullptr)); // Seed the random number generator

    // Strategy 3: Parallelize Initialization
    init_random(previous, current, N); // initialize previous array with a pattern

#ifdef OUTPUT
    print_to_pgm(previous, N, 0);
#endif

    /* Game of Life */

    // Strategy 5: Thread-Safe Timing using OpenMP's omp_get_wtime() 
    ts = omp_get_wtime();

    // Strategy 1: Parallelize Cell Updates
    for (t = 0; t < T; t++) {
#pragma omp parallel for private(i, j, nbrs) shared(current, previous)
        for (i = 1; i < N - 1; i++)
            for (j = 1; j < N - 1; j++) {
                nbrs = previous[i + 1][j + 1] + previous[i + 1][j] + previous[i + 1][j - 1] +
                    previous[i][j - 1] + previous[i][j + 1] +
                    previous[i - 1][j - 1] + previous[i - 1][j] + previous[i - 1][j + 1];
                if (nbrs == 3 || (previous[i][j] + nbrs == 3))
                    current[i][j] = 1;
                else
                    current[i][j] = 0;
            }

#ifdef OUTPUT
        print_to_pgm(current, N, t + 1);
#endif

#pragma omp parallel for private(i, j) shared(current, previous, swap)
        for (i = 0; i < N-1; i++)
            for (j = 0; j < N-1; j++)
                swap[i][j] = current[i][j];

        // Swap current array with the previous array
        swap = current;
        current = previous;
        previous = swap;
    }

    // Strategy 5: Thread-Safe Timing using OpenMP's omp_get_wtime()
    tf = omp_get_wtime();
    time = tf - ts;

    free_array(current, N);
    free_array(previous, N);
    std::cout << "GameOfLife: Size " << N << " Steps " << T << " Time " << time << std::endl;

#ifdef OUTPUT
    system(FINALIZE);
#endif
    return 0;
}
