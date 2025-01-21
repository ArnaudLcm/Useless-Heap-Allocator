# Multi-threaded heap allocator

Simple implementation of a heap allocator.

### Limitations
- Max number of threads: 128

### Requirements
- GCC


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