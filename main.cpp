#include <iostream>
#include "heap.hpp"

void debug_heap() {
    Chunk* curr = globalBase;
    std::cout << "--- HEAP STATE ---" << std::endl;
    if (!curr) std::cout << "Heap is empty (nullptr)" << std::endl;
    while (curr) {
        std::cout << "[Addr: " << curr 
                  << " | Size: " << curr->size 
                  << " | Free: " << (curr->isFree ? "YES" : "NO") 
                  << " | Next: " << curr->next << "]" << std::endl;
        curr = curr->next;
    }
    std::cout << "------------------" << std::endl;
}

int main()
{
    std::cout << "Testing myAllocator..." << std::endl;

    // Teste 1: Alocação Simples
    void* p1 = my_alloc(100);
    void* p2 = my_alloc(200);
    debug_heap();

    // Teste 2: Free e Reutilização (Splitting)
    std::cout << "\nFreeing p1 (100 bytes)..." << std::endl;
    my_free(p1);
    debug_heap();

    std::cout << "Asking 50 bytes (it should use memory freed from p1)..." << std::endl;
    void* p3 = my_alloc(50);
    debug_heap();

    // Teste 3: Coalescing (Fusão de vizinhos)
    std::cout << "\nFree p3 and p2 to test coalescing..." << std::endl;
    my_free(p3);
    my_free(p2);
    debug_heap(); // Aqui o Heap deve estar com um bloco unico livre ou vazio (se o trimming atuar)
    return 0;
}
