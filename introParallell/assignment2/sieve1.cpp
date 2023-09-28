#include <iostream>
#include <cmath>
#include <cstring>
#include <pthread.h>
#include <chrono>
#include <vector>
#include <algorithm>

using uint = unsigned int;

std::vector<bool> marked;
std::vector<uint> primes;

pthread_mutex_t primesMutex(PTHREAD_MUTEX_INITIALIZER);

void* sieve(void *args) {
    uint *range(reinterpret_cast<uint*>(args));

    uint start(range[0]);
    uint end(range[1]);
    uint max(range[2]);

    for (uint i(start); i <= end; ++i) {
        if (!marked.at(i)) {
            pthread_mutex_lock(&primesMutex);
            primes.push_back(i);
            pthread_mutex_unlock(&primesMutex);

            for (uint j(i * i); j <= max; j += i) {
                marked[j] = true;
            }
        }
    }

    return nullptr;
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
    } catch (std::exception) {
        usage(argv[0], 1);
    } if (threads < 1) {
        usage(argv[0], 1);
    }

    uint max;
    try {
        max = std::stoi(argv[2]);
    } catch (std::exception) {
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
    uint chunkSize((max - sqrtMax) / threads);
    pthread_t pThreads[threads];
    int thread_args[threads][3];

    thread_args[0][0] = 2;
    thread_args[0][1] = sqrtMax;
    thread_args[0][2] = max;

    sieve(thread_args[0]);

    for (uint i(0); i < threads; ++i) {
        uint start(sqrtMax + 1 + i * chunkSize);
        uint end((i == threads - 1) ? max : start + chunkSize - 1);

        thread_args[i][0] = start;
        thread_args[i][1] = end;
        thread_args[i][2] = max;

        pthread_create(&pThreads[i], nullptr, sieve, thread_args[i]);
    }

    for (uint i(0); i < threads; ++i) {
        pthread_join(pThreads[i], nullptr);
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