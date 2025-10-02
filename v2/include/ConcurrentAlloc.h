#pragma once
#include "ThreadCache.h"

void* ConcurrentAlloc(size_t size) {
    cout << "thread_id=" << std::this_thread::get_id() << ": " << pTLSThreadCache << endl;

    if(pTLSThreadCache == nullptr) {
        pTLSThreadCache = new ThreadCache;
        // to do
    }

    return pTLSThreadCache->Allocate(size);
}

void ConcurrentFree(void* obj, size_t size) {
    assert(obj);
    
    pTLSThreadCache->Deallocate(obj, size);
}