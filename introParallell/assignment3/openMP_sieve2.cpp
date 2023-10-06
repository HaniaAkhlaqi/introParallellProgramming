#include <iostream>
#include <cmath>
#include <cstring>
#include <chrono>
#include <vector>
#include <algorithm>
#include <omp.h> // Include OpenMP header

//openMP parallelization for sieve 2 program
//the whole range of numbers are parallelized from 2 to Max 

using uint = unsigned int;

std::vector<bool> marked;
std::vector<uint> primes;

void sieve(uint start, uint max) {
    uint sqrtMax = static_cast<uint>(sqrt(max));

    // Use OpenMP to parallelize the loop
    #pragma omp parallel for
    for (uint i = start; i <= max; ++i) {
        if (!marked[i]) {
            #pragma omp critical // Protect the primes vector from concurrent access
            primes.push_back(i);

            for (uint j = i * i; j <= max; j += i) {
                marked[j] = true;
            }
        }
    }
}

void usage(char* program, int code = 0) {
    std::cout << "Usage: " << program << " T M" << std::endl;
    std::cout << std::endl;
    std::cout << "  T: number of threads" << std::endl;
    std::cout << "  M: max prime" << std::endl;
    exit(code);
}

int main(int argc, char* argv[]) {
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
    }
    if (threads < 1) {
        usage(argv[0], 1);
    }

    uint max;
    try {
        max = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        usage(argv[0], 1);
    }
    if (max < 2) {
        usage(argv[0], 1);
    }

    // *** timing begins here ***
    auto start_time = std::chrono::system_clock::now();

    marked.resize(max + 1, false);
    marked[0] = true;
    marked[1] = true;

    // Call the parallelized sieve function
    sieve(2, max);

    std::sort(primes.begin(), primes.end());
    // for (int prime : primes) {
    //     std::cout << prime << " ";
    // }

    // *** timing ends here ***
    std::chrono::duration<double> duration = (std::chrono::system_clock::now() - start_time);
    std::cout << "Finished in " << duration.count() << " seconds (wall clock)." << std::endl;
    return 0;
}
