# Kth Smallest Element Finder Using QuickSort and IPC
This repository contains an implementation of a distributed system for finding the kth smallest element in an unsorted array. The system utilizes the QuickSort algorithm in conjunction with Inter-Process Communication (IPC) through pipes, leveraging multiple processes to enhance the efficiency and speed of the search. The solution is designed for Unix-based operating systems and showcases the use of process synchronization and communication in a parallel computing context.

## Features
#### QuickSort Algorithm:
- Efficiently sorts integers across multiple datasets in parallel.
#### IPC Mechanism: 
- Uses pipes for communication between parent and child processes, coordinating the sorting and selection tasks.
#### Parallel Processing: 
- Demonstrates multiprocessing to improve performance on multi-core processors.
#### Dynamic Data Handling: 
- Works with integers stored in separate files, each managed by a dedicated child process.

# Getting Started

## Requirements
- A Unix-based operating system (Linux or macOS)
- GCC compiler installed

## Installation
- Clone to your local machine
```
git clone https://github.com/MAXIMUSSCORP/kth-smallest-element.git
cd kth-smallest-element-finder
```
- Compile the program with GCC:
```
gcc -o kth_smallest kth_smallest.c
```

## Configuration
- Prepare input files in the same directory as the executable, The program expects integers to be sorted in files named input_1.txt, input_2.txt, ..., integers are seperated by whatever the user wants eg include single space, new line, double space etc.
- Sepcify under Define block number of files and number of integers per file
```
#define READ 0
#define WRITE 1
#define NUM_CHILDREN 5
#define ARRAY_SIZE 5 // Assuming each file has 5 integers
```
each NUM_CHILDREN takes 1 file
ARRAY_SIZE takes number of integers per file

## Usage
After compilation, you can run the program by executing the parallel_quicksort binary. REMEMBER The program expects integers to be sorted in files named input_1.txt, input_2.txt, ..., corresponding to each child process it spawns.
```
./kth_smallest
```

# License
This project is licensed under the MIT License - see the [LICENSE.md](https://github.com/MAXIMUSSCORP/kth-smallest-element/blob/main/LICENSE) file for details.
