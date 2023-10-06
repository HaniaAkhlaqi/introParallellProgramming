#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <sys/time.h>

#define FINALIZE "\
ffmpeg -y -start_number 0 -i out%d.pgm output.gif\n\
rm *pgm\n\
"
// Allocates a 2D integer array of size N x N and initializes it with zeros.
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

// Deallocates memory previously allocated for a 2D integer array.
static void free_array(int** array, int N) {
    for (int i = 0; i < N; i++)
        delete[] array[i];
    delete[] array;
}

// Randomly initializes two 2D arrays array1 and array2 with a pattern of ones in approximately 10% of the cells.
static void init_random(int** array1, int** array2, int N) {
    int i, pos;

    for (i = 0; i < (N * N) / 10; i++) {
        pos = rand() % ((N - 2) * (N - 2));
        array1[pos % (N - 2) + 1][pos / (N - 2) + 1] = 1;
        array2[pos % (N - 2) + 1][pos / (N - 2) + 1] = 1;
    }
}

// Conditionally compiled when OUTPUT is defined, creates a PGM (Portable Gray Map) image file representing the state of the 2D array at a given time step t.
#ifdef OUTPUT
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
#endif

int main(int argc, char* argv[]) {
    int N;             // array dimensions
    int T;             // time steps
    int** current, ** previous; // arrays - one for the current timestep, one for the previous timestep
    int** swap;        // array pointer
    int t, i, j, nbrs; // helper variables

    double time;       // variables for timing
    struct timeval ts, tf;

    /* Read input arguments */
    if (argc != 3) {
        std::cerr << "Usage: ./exec ArraySize TimeSteps\n";
        exit(-1);
    }
    else {
        N = atoi(argv[1]);
        T = atoi(argv[2]);
    }

    /* Allocate and initialize matrices */
    current = allocate_array(N);         // allocate array for the current time step
    previous = allocate_array(N);        // allocate array for the previous time step

    std::srand(std::time(nullptr)); // Seed the random number generator
    init_random(previous, current, N); // initialize previous array with a pattern

#ifdef OUTPUT
    print_to_pgm(previous, N, 0);
#endif

    /* Game of Life */

    gettimeofday(&ts, NULL);
    for (t = 0; t < T; t++) {
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
        // Swap current array with the previous array
        swap = current;
        current = previous;
        previous = swap;
    }
    gettimeofday(&tf, NULL);
    time = (tf.tv_sec - ts.tv_sec) + (tf.tv_usec - ts.tv_usec) * 0.000001;

    free_array(current, N);
    free_array(previous, N);
    std::cout << "GameOfLife: Size " << N << " Steps " << T << " Time " << time << std::endl;

#ifdef OUTPUT
    system(FINALIZE);
#endif
    return 0;
}
