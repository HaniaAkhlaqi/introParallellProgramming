#include <iostream>
#include <cmath>
#include <cstring>
#include <pthread.h>
#include <chrono>
#include <vector>
#include <queue>
#include <functional>

using uint = unsigned int;

std::queue<std::function<void()>> taskQueue;
std::vector<bool> marked;

pthread_mutex_t taskQueueMutex(PTHREAD_MUTEX_INITIALIZER);
pthread_cond_t taskQueueCond(PTHREAD_COND_INITIALIZER);
bool allTasksCompleted(false);

void markMultiples(uint i, uint max) {
    for (uint j(2*i); j <= max; j += i) {
        marked[j] = true;
    }
}

void* worker(void* arg) {
    while (true) {
        std::function<void()> task;

        // Fetch a task from the queue
        pthread_mutex_lock(&taskQueueMutex);
        while (taskQueue.empty() && !allTasksCompleted) {
            pthread_cond_wait(&taskQueueCond, &taskQueueMutex);
        }

        if (taskQueue.empty() && allTasksCompleted) {
            pthread_mutex_unlock(&taskQueueMutex);
            break;
        }

        task = taskQueue.front();
        taskQueue.pop();
        pthread_mutex_unlock(&taskQueueMutex);

        // Execute the task
        task();
    }
    return nullptr;
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
    pthread_t pThreads[threads];

    // Create worker threads
    for (uint i(0); i < threads; ++i) {
        pthread_create(&pThreads[i], nullptr, worker, nullptr);
    }

    // Generate and enqueue tasks
    for (uint i(2); i <= sqrtMax; ++i) {
        if (!marked.at(i)) {
            std::function<void()> task([i, max]() {
                markMultiples(i, max);
            });
            pthread_mutex_lock(&taskQueueMutex);
            taskQueue.push(task);
            pthread_cond_signal(&taskQueueCond); // Notify a waiting thread to pick up the task
            pthread_mutex_unlock(&taskQueueMutex);
        }
    }

    // Signal all tasks are completed
    pthread_mutex_lock(&taskQueueMutex);
    allTasksCompleted = true;
    pthread_cond_broadcast(&taskQueueCond); // Signal all worker threads to exit
    pthread_mutex_unlock(&taskQueueMutex);

    for (uint i(0); i < threads; ++i) {
        pthread_join(pThreads[i], nullptr);
    }

    // Print prime numbers
    for (uint i(2); i <= max; ++i) {
        if (!marked.at(i)) {
            std::cout << i << " ";
        }
    }

    // *** timing ends here ***
    std::chrono::duration<double> duration((std::chrono::system_clock::now() - start_time));
    std::cout << "Finished in " << duration.count() << " seconds (wall clock)." << std::endl;
    return 0;
}