# Linux Custom Heap Allocator

A low-level memory management system developed in C++ that implements a custom heap allocator from scratch, bypassing the standard `malloc` and `free`.

_the hardest project that i have ever made_ 

This project interacts directly with the Linux Kernel via the `sbrk` system call to manage the program break, implementing a doubly linked list architecture to track memory chunks, fragmentation, and coalescing.

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

---

## About the Project
A fundamental systems programming project designed to demystify (for me) how dynamic memory allocation works "under the hood." Instead of using the standard library, this implementation manages raw memory blocks, handles metadata headers, and implements manual memory reclamation.

The engine uses a **Doubly Linked List** of "Chunks" (metadata headers), allowing for efficient bidirectional traversal. This architecture enables the implementation of advanced features like immediate coalescing of adjacent free blocks and intelligent heap trimming.

**Main Objectives:**
* Implement **Custom Malloc & Free** using the `sbrk` syscall.
* Implement **Memory Splitting** to reduce internal fragmentation.
* Implement **Immediate Coalescing** (merging) of adjacent free blocks.
* Implement a **Realloc** with in-place expansion and data migration logic.
* Understand **Memory Alignment**, pointer arithmetic, dynamic memory storage and Stack versus Heap.
* Utilize a data structure (in this case, the Doubly Linked List).

## Technologies

* **Language:** ISO C++17
* **API:** Linux System Calls (`unistd.h`)
* **Concepts:** Doubly Linked Lists, Pointer Arithmetic, Heap Management.

## How to Build and Run

### Prerequisites
Ensure you are running a Linux environment, as the project relies on the `sbrk` system call.

#### Compiling
To compile the allocator and the debug suite, run:

```bash
g++ main.cpp heap.cpp -o allocator

./allocator 
```

## Memory Management Logic

### Allocation (First-Fit (maybe change later to best-fit))
* **Behavior**: Scans the linked list for the first free chunk that fits the requested size.
* **Splitting**: If a found chunk is significantly larger than requested, it is split into two: one occupied and one free remainder.

### Freeing & Coalescing
* **Behavior**: Marks chunks as free and immediately checks `prev` and `next` pointers.
* **Logic**: If neighbors are free, they are merged into a single larger chunk to prevent external fragmentation.
* **Heap Trimming**: If the last chunk in the heap is free and exceeds a 4KB threshold, it is released back to the OS via negative `sbrk`.



### Reallocation (Smart Realloc)
* **Shrink**: If the new size is smaller, the block is split in-place.
* **Expansion**: If the new size is larger, it first attempts to "swallow" the next chunk if it is free and has enough space.
* **Migration**: If in-place expansion is impossible, it allocates a new block, moves data via `memcpy`, and frees the old one.

---

## Project Structure

```Markdown
📁 Project
├── 📄 heap.hpp         # Declarations, Chunk struct, and Global State
├── 📄 heap.cpp         # Implementation of my_alloc, my_free, and my_realloc
├── 📄 main.cpp         # Debug suite and heap state visualization
└── 📄 .gitignore       # Prevents tracking of binaries
```

## Resources
Key resources used for understanding Memory Alignment, Heap architecture and Syscalls:

* **Man sbrk(2)**: [Linux manual page for program break manipulation](https://man7.org/linux/man-pages/man2/sbrk.2.html)
* **Man brk(2)**: [Linux manual page for data segment management](https://man7.org/linux/man-pages/man2/brk.2.html)
* **Build your own Memory Allocator**: [Giovanni Iannaccone (Medium)](https://medium.com/@iannacconegiovanni444/build-your-own-memory-allocator-a5f58763083d)
* **Low Level Learning**: [i wrote my own memory allocator in C to prove a point](https://www.youtube.com/watch?v=CulF4YQt6zA)
* **LaurieWired**: [Mastering Memory: Allocation Techniques in C, C++, and ARM Assembly](https://www.youtube.com/watch?v=HlUBE70h2C0)


## Reminder

This project was developed for educational purposes to learn low-level memory management and the interface between user-space and the Linux Kernel. Modern allocators are highly optimized for multithreading and safety, the goal here was to experience through the manual pointer arithmetic and linked list costuring to understand how memory actually behaves before it reaches higher-level abstractions.

Paraphrasing LowLevelLearning:

This memory allocator is safe? no

Is optimized? Probably no

I have fun with it? yes

I have learn with it? a lot.
