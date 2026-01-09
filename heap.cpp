#include "heap.hpp"
#include <cstring>
#include <errno.h>
#include <iostream>
#include <unistd.h>

Chunk* globalBase = nullptr;
Chunk* globalLast = nullptr;
constexpr const size_t SIZE_4KB = 4096;

size_t align(size_t n)
{
    return (n + sizeof(void*) - 1) & ~(sizeof(void*) - 1);
}

Chunk* findFreeBlock(std::size_t space)
{
    Chunk* ptr = globalBase;
    // First Fit
    while(ptr != nullptr)
    {
        if (ptr->isFree && ptr->size >= space)
        {
            // fragmentates if chunk is too big
            if (ptr->size - space >= sizeof(Chunk) + 1)
            {
                // TODO: FIX LOGIC
                // [A] <-> [ptr] <-> [B]
                Chunk* newchunk = reinterpret_cast<Chunk*>((reinterpret_cast<uint8_t*>(ptr) + space + sizeof(Chunk)));
                newchunk->size = ptr->size - space - sizeof(Chunk);
                newchunk->prev = ptr;
                newchunk->next = ptr->next;
                newchunk->isFree = true; 

                if (newchunk->next == nullptr) { globalLast = newchunk; }
                
                ptr->next = newchunk;
                ptr->size = space;
                ptr->next->prev = newchunk;
            }
            ptr->isFree = false;
            return ptr; 
        }
        ptr = ptr->next; 
    }
    ptr = nullptr;
    return ptr; 
}

void* requestSpace(std::size_t space)
{
    std::size_t requestedSize =  space + sizeof(Chunk);
    Chunk* chunk = static_cast<Chunk*>(sbrk(align(requestedSize)));
    if (chunk == (void*) -1) { std::cerr << "unable to allocate memory\n" << errno; return nullptr; }
    
    chunk->size = space;
    chunk->isFree = false;
    chunk->next = nullptr;
    // if first time, update global, if not, save in next and update globalLast 
    if (globalBase == nullptr )
    {
        globalBase = chunk;
        chunk->prev = nullptr; 
        globalLast = globalBase;
    }
    else 
    {
        globalLast->next = chunk;
        chunk->prev = globalLast;
        globalLast = chunk;
    }
    return chunk; 
}

void* my_alloc(std::size_t size)
{
    std::size_t alignedSize = align(size);
    Chunk* chunk = static_cast<Chunk*>(findFreeBlock(alignedSize));
    if (chunk == nullptr)
    {
        Chunk* chunk = static_cast<Chunk*>(requestSpace(alignedSize));
        return chunk + 1;
    }
    return chunk + 1;
}

void my_free(void* ptr)
{
    if (ptr == nullptr) { return; }
    // -1 because we are subtracting the header
    // like: - sizeof(Chunk). pointer arithmetic
    Chunk* chunk = static_cast<Chunk*>(ptr) - 1;
    chunk->isFree = true;
 
    // if next has something
    while(chunk->next != nullptr && chunk->next->isFree)
    {
        // Coalesce the next and the prev
        chunk->size += sizeof(Chunk) + chunk->next->size;
        chunk->next = chunk->next->next;
        // if have next, set the next to the new chunk
        if (chunk->next != nullptr)
        {
            chunk->next->prev = chunk;
        }
        else { globalLast = chunk; }
    }
    // Trimming
    if (chunk == globalLast)
    {
        if (chunk->size >= SIZE_4KB)
        {
            // previous points to nothing
            // in case freeing just a single chunk of memory
            if (chunk->prev != nullptr)
            {
                chunk->prev->next = nullptr;
                globalLast = chunk->prev;
            }
            else // if previous is null, we set our global to null 
            {
                globalBase = nullptr; 
                globalLast = nullptr;
            }
            // removes chunk of data
            chunk->next = nullptr;
            sbrk(-(chunk->size + sizeof(Chunk)));
            return;
        }
    }
    // Coalesce behind memory 
    if (chunk->prev == nullptr) {globalBase = chunk;}
    // same thing, but linking the back one 
    if (chunk->prev != nullptr && chunk->prev->isFree)
    {
        // A                   C 
        chunk->prev->next = chunk->next;
        // checks if next exists
        if (chunk->next != nullptr)
        {
            chunk->next->prev = chunk->prev;
        }
        else{ globalLast = chunk->prev; }
        chunk->prev->size += sizeof(Chunk) + chunk->size;
    }

}

void* my_calloc(size_t n, size_t size)
{
    // overflow checking 
    // size_t * n = max 
    // so the max value of size_t is max / n 
    // since size_t is unsigned, his max value is (size_t) -1
    if (n != 0 && size > (size_t)-1/n) { return nullptr; }

    size_t totalSize = n * size; 
    void* ptr = my_alloc(totalSize);
    
    if (!ptr) { return nullptr; }

    memset(ptr, 0, totalSize);
    return ptr; 
}

void* my_realloc(void* ptr, std::size_t size)
{   
    // case ptr is NULL 
    if (!ptr) { return my_alloc(size); }

    // getting headers
    Chunk* chunk = static_cast<Chunk*>(ptr) - 1;
    // already big enough
    if (chunk->size == size) { return ptr; }
    // increase size 
    if (chunk->size < size)
    {
        // if has something in front
        if (chunk->next != nullptr && chunk->next->isFree == true && (chunk->size + sizeof(Chunk) + chunk->next->size) >= size)
        {
            chunk->size += sizeof(Chunk) + chunk->next->size;
            if (chunk->next->next == nullptr)
            {
                globalLast = chunk;
            }
            // A               C 
            chunk->next = chunk->next->next;
            // [A CHUNK] <-> [B CHUNK NEXT] <-> [C CHUNK NEXT NEXT]
            // [A CHUNK] <-> [C CHUNK->NEXT]
            if (chunk->next != nullptr) { chunk->next->prev = chunk; }
            return ptr; 
        }
        // if not, resize by getting more memory 
        else 
        {
            void* moreMem = my_alloc(size);
            memcpy(moreMem, ptr, chunk->size);
            my_free(chunk + 1); // header and data
            return moreMem; 
        }
    }
    // decrease size
    // (1) [A] <-> [cut and chunk] ------>|  points to B
    // then, this happens
    // (2) [A] <-> [chunk] <-> [cut] <-> [B]
    Chunk* cut = reinterpret_cast<Chunk*>((uint8_t*)ptr + size);
    cut->size = chunk->size - size - sizeof(Chunk);
    chunk->size = size;
   
    // (1)
    cut->next = chunk->next;
    
    // (2)
    if (cut->next != nullptr)
    {
        cut->next->prev = cut;
    }
    cut->prev = chunk;
    chunk->next = cut; 
    
    my_free(cut + 1);
    return ptr;
}

