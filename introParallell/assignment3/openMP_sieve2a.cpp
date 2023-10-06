#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <omp.h> // Include OpenMP header

//parallelization of sieve 2 program using OPENMP
//primarily parallelize the loops within the `main` function, as the main computation happens there. The first thread will calculate primes from 2 to `sqrt(max)`, and then we will use OpenMP to parallelize the loops to calculate primes from `sqrt(max) + 1` to `max`.

using uint = unsigned int;

std::vector<bool> marked;
std::vector<uint> primes;

void sieve(uint start, uint max) {
    for (uint i(start); i <= max; i++) {
        if (!marked.at(i)) {
            #pragma omp critical // Use a critical section to ensure thread safety
            {
                primes.push_back(i);
            }

            for (uint j(i * i); j <= max; j += i) {
                marked[j] = true;
            }
        }
    }
}

void usage(char *program, int code = 0) {
    std::cout << "Usage: " << program << " T M" << std::endl;
    std::cout << std::endl;
    std::cout << "  T: number of threads" << std::endl;
    std::cout << "  M: max prime" << std::endl;
    exit(code);
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        usage(argv[0]);
    } else if (argc != 3) {
        usage(argv[0], 1);
    }

    uint threads;
    try {
        threads = std::stoi(argv[1]);
    } catch (const std::exception& e) {
        usage(argv[0], 1);
    } if (threads < 1) {
        usage(argv[0], 1);
    }

    uint max;
    try {
        max = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        usage(argv[0], 1);
    } if (max < 2) {
        usage(argv[0], 1);
    }

    // *** timing begins here ***
    auto start_time(std::chrono::system_clock::now());

    marked.resize(max + 1, false);
    marked[0] = true;
    marked[1] = true;

    uint sqrtMax(static_cast<uint>(sqrt(max)));

    // Calculate primes from 2 to sqrt(max) sequentially
    sieve(2, sqrtMax);

#pragma omp parallel num_threads(threads)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        // Calculate primes from sqrt(max) + 1 to max in parallel
        uint start = sqrtMax + 1 + (max - sqrtMax - 1) * thread_id / num_threads;
        uint end = sqrtMax + 1 + (max - sqrtMax - 1) * (thread_id + 1) / num_threads - 1;

        sieve(start, end);
    }

    std::sort(primes.begin(), primes.end());
    for (int prime : primes) {
        std::cout << prime << " ";
    }

    // *** timing ends here ***
    std::chrono::duration<double> duration((std::chrono::system_clock::now() - start_time));
    std::cout << "Finished in " << duration.count() << " seconds (wall clock)." << std::endl;
    return 0;
}
