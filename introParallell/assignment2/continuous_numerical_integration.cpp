#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <cstdlib>
#include <cstring>

const double PI = 3.14159265358979323846;

// Function to calculate the value of f(x)
double func(double x) {
    return 4.0 / (1.0 + x * x);
}

// Data parallel numerical integration using trapezoidal rule
double parallelIntegrate(double a, double b, int num_trapezoids, int num_threads) {
    double integral = 0.0;
    double delta_x = (b - a) / num_trapezoids; // trapezoids intersection with x-axis aka stepsize 

    // Create and launch threads
    std::vector<std::thread> threads(num_threads);

    // Calculate the number of trapezoids per thread
    int trapezoids_per_thread = num_trapezoids / num_threads;

    // Define a vector to store the partial results for each thread
    std::vector<double> partial_results(num_threads, 0.0);

    // *** timing begins here ***
    auto start_time = std::chrono::system_clock::now();
    for (int i = 0; i < num_threads; i++) {
        int start = i * trapezoids_per_thread;  // Start index for the thread's trapezoids
        int end = (i == num_threads - 1) ? num_trapezoids : (i + 1) * trapezoids_per_thread; // End index

        threads[i] = std::thread([&](int thread_id, int start, int end) {
            double local_sum = 0.0;

            for (int j = start; j < end; j++) {
                double x0 = a + j * delta_x; // Lower bound of trapezoid
                double x1 = a + (j + 1) * delta_x; // Upper bound of trapezoid
                local_sum += (func(x0) + func(x1)) / 2.0 * delta_x;
            }

            partial_results[thread_id] = local_sum;
        }, i, start, end);
    }

    // Wait for all threads to finish
    for (std::thread& t : threads) {
        t.join();
    }

    // Sum up the partial results
    for (double result : partial_results) {
        integral += result;
    }

    std::chrono::duration<double> duration =
        (std::chrono::system_clock::now() - start_time);
    // *** timing ends here ***

    std::cout << "Finished in " << duration.count() << " seconds (wall clock)." << std::endl;

    return integral;
}

int main(int argc, char* argv[]) {
    if (argc != 4 || strcmp(argv[1], "-h") == 0) {
        std::cout << "Usage: " << argv[0] << " <num_threads> <num_trapezoids> -h" << std::endl;
        return 1;
    }

    int num_threads = atoi(argv[1]);
    int num_trapezoids = atoi(argv[2]);

    double a = 0.0;
    double b = 1.0;

    double result = parallelIntegrate(a, b, num_trapezoids, num_threads);

    std::cout << "Estimated integral: " << result << std::endl;
    std::cout << "Expected integral (π): " << PI << std::endl;

    return 0;
}