#include "heap.hpp"
#include <errno.h>
#include <iostream>
#include <unistd.h>

Chunk* globalBase = nullptr;
Chunk* globalLast = nullptr;

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
    // if next has something
    if (chunk->next != nullptr && chunk->next->isFree)
    {
        chunk->size += sizeof(Chunk) + chunk->next->size;
        chunk->next = chunk->next->next;
    }
    chunk->isFree = true;

}

