#!/bin/bash

# Set the compiler and compile flags
COMPILER=g++
FLAGS="-std=c++11 -Wall -fopenmp"

# Set the output file name
OUTPUT_FILE="sieve3_results.txt"

# Define the thread counts and MAX values
THREADS=("1" "10" "100" "1000" "10000")
MAX_VALUES=("1" "10" "100" "1000" "10000" "1000000")

# Compile the C++ program
$COMPILER $FLAGS -o sieve3_openmp openmp_sieve3.cpp

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful."
else
    echo "Compilation failed. Exiting."
    exit 1
fi

# Run the program with different thread counts and MAX values
for threads in "${THREADS[@]}"; do
    for max in "${MAX_VALUES[@]}"; do
        echo "Running with $threads threads and MAX = $max..."
        ./sieve3_openmp $threads $max >> $OUTPUT_FILE
        echo "Threads: $threads, MAX: $max" >> $OUTPUT_FILE
    done
done

echo "All runs completed. Results saved in $OUTPUT_FILE."
