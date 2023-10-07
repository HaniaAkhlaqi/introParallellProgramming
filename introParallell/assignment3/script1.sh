#!/bin/bash

# Set the compiler and compile flags
COMPILER=g++
FLAGS="-std=c++11 -Wall -fopenmp"

# Set the output file name
OUTPUT_FILE="gameoflife_analysis.txt"

# Define the thread counts and MAX values
THREADS=("1" "2" "4" "8" "16")
MAX_VALUES=("64" "1024" "4096")
STEPS=("1000" "2000")

# Compile the C++ program
$COMPILER $FLAGS -o openmp_gameoflife_analysis openmp_game_of_life2.cpp

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful."
else
    echo "Compilation failed. Exiting."
    exit 1
fi

# Run the program with different thread counts and MAX values
for threads in "${THREADS[@]}"; do
    for size in "${MAX_VALUES[@]}"; do
        for step in "${STEPS[@]}"; do
        echo "Running with $threads threads and size = $size and steps $step ..."
        ./openmp_gameoflife_analysis $step $size $threads >> $OUTPUT_FILE
        echo "Size: $size, Steps: $step, Threads: $threads" >> $OUTPUT_FILE
        done
    done
done

echo "All runs completed. Results saved in $OUTPUT_FILE."
