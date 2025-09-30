#pragma once
#include "ThreadCache.h"

void* ConcurrentAlloc(size_t size) {
    if(pTLSThreadCache == nullptr) {
        pTLSThreadCache = new ThreadCache;
        // to do
    }

    return pTLSThreadCache->Allocate(size);
}

void ConcurrentFree(void* ptr) {
    // pTLSThreadCache->Deallocate(ptr);
}