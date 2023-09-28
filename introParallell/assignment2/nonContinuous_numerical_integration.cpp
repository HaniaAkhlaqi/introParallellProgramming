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

// Parallel numerical integration using trapezoidal rule menaing the integration interval is divided into subrange that 
double parallelIntegrate(double a, double b, int num_trapezoids, int num_threads) {
    double integral = 0.0;
    double delta_x = (b - a) / num_trapezoids; // calculating the stepsize for each trapeze. 
    
    // Create a vector to store the partial results 
    std::vector<double> partial_results(num_threads, 0.0);

    // Function to calculate the integral in a range [start, end)
    auto integrateRange = [&](int thread_id, int start, int end) {
        double local_sum = 0.0;
        for (int i = start; i < end; i++) {
            double x0 = a + i * delta_x; //lowerbound in a  trapeze, start of subinterval
            double x1 = a + (i + 1) * delta_x; //upperbound in a trapeze, end of subinterval 
            local_sum += (func(x0) + func(x1)) / 2.0 * delta_x; //calculate the area of each trapezoid
        }
        partial_results[thread_id] = local_sum;
    };

    // Create and launch threads
    std::vector<std::thread> threads(num_threads);

    // Calculate the number of trapezoids per thread
    int trapezoids_per_thread = num_trapezoids / num_threads;

    // *** timing begins here ***
    auto start_time = std::chrono::system_clock::now();
    for (int i = 0; i < num_threads; i++) { 
        int start = i * trapezoids_per_thread;  //lowerbouand for a thread aka the index of the first trapezoid that the thread should process
        int end = (i == num_threads - 1) ? num_trapezoids : (i + 1) * trapezoids_per_thread; //upperbound for a thread aka the index of the trapezoid immediately after the last one the thread should process
        threads[i] = std::thread(integrateRange, i, start, end); //each thread does a partial integral 
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
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