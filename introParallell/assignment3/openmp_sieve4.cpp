#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>
#include <omp.h>

void sequentialSieve(std::vector<bool> &isPrime, int max) {
    int sqrtMax = static_cast<int>(std::sqrt(max));
    for (int i = 2; i <= sqrtMax; ++i) {
        if (isPrime[i]) {
            for (int j = i * i; j <= max; j += i) {
                isPrime[j] = false;
            }
        }
    }
}

void parallelSieve(std::vector<bool> &isPrime, int max, int numThreads) {
    int sqrtMax = static_cast<int>(std::sqrt(max));
    int chunkSize = (max - sqrtMax) / numThreads;
    
    #pragma omp parallel num_threads(numThreads)
    {
        int threadId = omp_get_thread_num();
        int start = sqrtMax + 1 + threadId * chunkSize; //start if a threads chunk 
        int end = (threadId == numThreads - 1) ? max : start + chunkSize - 1; //end of a threads chunk
        
        for (int i = 2; i <= sqrtMax; ++i) { //this is because we use sequentially calculated primes as seed 
            if (isPrime[i]) {
                int startMultiple = std::max(i * i, (start / i) * i); // Calculate the first multiple to mark
                for (int j = startMultiple; j <= end; j += i) {
                    isPrime[j] = false; //marking the multiples as non prime in this chunk
                }
            }
            //// Barrier synchronization to ensure all threads have processed the current 'i' before moving to the next 'i
            #pragma omp barrier
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

    int numThreads;
    try {
        numThreads= std::stoi(argv[1]);
    } catch (const std::exception& e) {
        usage(argv[0], 1);
    } if (numThreads < 1) {
        usage(argv[0], 1);
    }

    int Max;
    try {
        Max = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        usage(argv[0], 1);
    } if (Max < 2) {
        usage(argv[0], 1);
    }

    
    // Create a vector to store whether each number is prime or not
    std::vector<bool> isPrime(Max + 1, true);
    
    // Mark 0 and 1 as non-prime
    isPrime[0] = isPrime[1] = false;
    
    double startTime = omp_get_wtime();
    
    // Sequentially compute primes up to sqrt(Max)
    sequentialSieve(isPrime, static_cast<int>(std::sqrt(Max)));
    
    // Parallelize the Sieve of Eratosthenes for the remaining range

    parallelSieve(isPrime, Max, numThreads);
    
    double endTime = omp_get_wtime();
    
    
    // // Print the prime numbers
    // for (int i = 2; i <= Max; ++i) {
    //     if (isPrime[i]) {
    //         std::cout << i << " ";
    //     }
    // }

    std::cout << "Prime numbers up to " << Max << " computed in " << endTime - startTime << " seconds." << std::endl;

    return 0;
}
