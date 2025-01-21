<div align='center'>

  <h1>Useless Heap Allocator</h1>
  <p>A fairly simple heap allocator.</p>

<h4>
  
  <span> · </span> <a href="https://github.com/ArnaudLcm/Useless-Heap-Allocator/issues"> Report Bug </a>
  <span> · </span> <a href="https://github.com/ArnaudLcm/Useless-Heap-Allocator/issues"> Request Feature </a>
   · 
</h4>
</div>

## About

The goal of this project is to implement a simple heap allocator designed for concurrent use with multiple threads. To avoid relying on a global mutex, it uses **arenas** to handle allocations efficiently in a multi-threaded environment.


**Current limitations:**
- Max number of threads (if heap size is enough): 128


> [!CAUTION]
> There is currently no mechanism to avoid heap overflow. 

> [!WARNING]
> This is not a production ready library. Use it at your own risk.

## Getting Started

### Dependencies
- GCC 5.x or newer 
- Make

### Build the library

To build the library, please execute:
```bash
make
```

## Features
 - Use arenas to handle multi-threaded allocations.
 - Use sbrk and mmap syscalls to setup the initial heap.
 - Use stacks to have O(1) access to free nodes.
 - Usage of 3 different bin type (small, medium and unsorted) to improve memory allocation efficiency.

## Notes

### Notes - Design
*Bunch of notes related to the design of this heap allocator. Therefore, it is obviously messy.*

- We manage the heap size with sbrk syscall
- Header size: 4 bytes
- Header content: 
      - 0-28 bits: Payload size
      - 29-32: State -> (000: Free, 001: Used)
- Initially, first block = heap size
- We ensure alignment with padding
- Free chunk of data are tracked through free linked list called bins
- We would like to implement a best fit policy: We find the smallest spot where we can fit the asked memory allocation
- Coalescing: When a set of chunks are free, we merge them
- Bins: As mentionned, we keep track of free chunks through a free list. Instead of having only one free list, we will be using 3 different free lists:
    - Small bin: For free chunks of 512 bytes
    - Medium Bin: For free chunks of 1024 bytes
    - Unsorted bin: For all other free chunks
    
  *We should have more free lists like the glibc does to be more exhaustive but for now let's remain simple.*

- Free list managements: They are allocated statically and we manage which node is free through a stack


### Notes - Features I could implement

*List of features I could consider to improve the current state of the allocator*:

- Use XOR linked lists
- Like GLibC allocator, have 128 bins with 64 small bins, 63 large bins and 1 unsorted bin
- Add mechanisms to prevent heap overflow
- Check if I can use atomics instead of a mutex for arena allocations