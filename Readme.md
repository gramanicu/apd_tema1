# APD, 1st Homework - "Parallel fractal generation using Mandelbrot and Julia Sets"

The first homework for the Parallel & Distributed Algorithms Course. 

## Problem solution

Firstly, I wanted to remove all global variables from the program and move struct declarations and helper functions to a header file.

I added a struct that holds the program arguments (input, output and the number of threads) and a struct that will holds all the arguments that need to be passed to each thread. Some are arguments from the main function, others are parameters that need to be shared between threads:

1. From main:
    - the id of the thread
    - the program arguemts
    - the barrier used for sync
2. Between threads:
    - the result matrix and its dimensions
    - the parameters read from the input files

For each algorithm, the steps taken to parallelize it were:

1. Read the data in a single thread and allocate memory for the result (T0)
2. Sync
3. Compute the portion of the matrix/loop each thread should compute (start/end) and run the algorithms
4. Sync
5. Compute start/end again and transform the result.
6. Sync
7. Write the data to the output file and free the memory for the result, in a single thread (T0), and then continue with step 1 for the 2nd algorithm.

© 2020 Grama Nicolae
