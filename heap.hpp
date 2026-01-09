#ifndef HEAP_H
#define HEAP_H

#include <cstdint>
#include <unistd.h>


struct Chunk
{
    std::size_t size;    // size of the chunk
    bool isFree = false; // is free?
    Chunk* next;   // next chunk 
    Chunk* prev = nullptr; // previous chunk
    void* data; // data
};
extern Chunk* globalBase;
extern Chunk* globalLast;

void* requestSpace(std::size_t space);
Chunk* findFreeBlock(std::size_t space);
void* my_alloc(std::size_t size); 
void my_free(void* ptr);
#endif
