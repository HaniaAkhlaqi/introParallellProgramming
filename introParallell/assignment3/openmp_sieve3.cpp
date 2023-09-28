#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#include <queue>
#include <functional>
#include <chrono>
#include <omp.h>

// parallelize the generation of tasks for numbers up to sqrt(max) &
///only parallelizes the calculation of primes from sqrt(max)+1 to max using OpenMP

using uint = unsigned int;

std::queue<std::function<void()>> taskQueue;
std::vector<bool> marked;

void markMultiples(uint i, uint max) {
    for (uint j(2*i); j <= max; j += i) {
        marked[j] = true;
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
    auto start_time = std::chrono::system_clock::now();

    marked.resize(max + 1, false);
    marked[0] = true;
    marked[1] = true;

    uint sqrtMax = static_cast<uint>(sqrt(max));

    // Generate and enqueue tasks for numbers up to sqrt(max)
    #pragma omp parallel num_threads(threads)
    {
        #pragma omp for schedule(dynamic)
        for (uint i = 2; i <= sqrtMax; ++i) {
            if (!marked[i]) {
                #pragma omp critical
                {
                    std::function<void()> task = [i, max]() {
                        markMultiples(i,max);
                    };
                    taskQueue.push(task);
                }
            }
        }
    }


    // Process tasks in parallel
    #pragma omp parallel num_threads(threads)
    {
        while (true) {
            std::function<void()> task;

            #pragma omp critical
            {
                if (!taskQueue.empty()) {
                    task = taskQueue.front();
                    taskQueue.pop();
                }
            }

            if (!task) {
                break;
            }

            // Execute the task
            task();
        }
    }

    // // Print prime numbers
    // for (uint i(2); i <= max; ++i) {
    //     if (!marked.at(i)) {
    //         std::cout << i << " ";
    //     }
    // }
    // Calculate and print the execution time
    auto end_time = std::chrono::system_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "Finished in " << duration.count() << " seconds (wall clock)." << std::endl;

    return 0;
}
