# Memory Management System

This project implements a custom memory allocation system using a Buddy Allocation Algorithm, designed for efficient memory management in environments with constrained resources.
The system consists of several key files and functions that enable memory allocation, deallocation, resizing, and visualization of memory usage.
The memory allocator is accompanied by a test program to validate its functionality.


Buddy Memory Allocation:
- Efficient memory allocation and deallocation using a hierarchical buddy allocation strategy.
- Memory blocks are split or merged as needed to minimize fragmentation.


Custom Memory Management Functions:
mem_alloc(): Allocate memory dynamically.
mem_free(): Free previously allocated memory.
mem_realloc(): Resize allocated memory.
mem_dump(): Print the current state of the memory heap.


Simulation of Memory Behavior:
A simulated memory heap is implemented using static arrays and linked lists.
Testing Suite:
- A test program validates functionality for memory allocation, reallocation, deallocation, and fragmentation handling.
