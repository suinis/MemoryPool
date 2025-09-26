// MIT License

// Copyright (c) 2025 suinis

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

constexpr int MEMORY_POOL_NUM = 64;
constexpr int SLOT_BASE_SIZE = 8;
constexpr int MAX_SLOT_SIZE = MEMORY_POOL_NUM * SLOT_BASE_SIZE;

struct Slot {
    Slot* next;
};

class MemoryPool {
public:
    MemoryPool(size_t BlockSize = 4096) : BlockSize_(BlockSize) {}
    ~MemoryPool();

    void init(size_t slot_size);
    void* allocate();
    void deallocate(void* ptr);

private:
    void allocateNewBlock();

private:
    int BlockSize_;
    int SlotSize_;
    Slot* firstBlock_;
    Slot* curSlot_;
    Slot* freeList_;
    Slot* lastSlot_;

};

// 采用单例模式
class HashBucket {
public:
    void initMemoryPool();
    MemoryPool& getMemoryPool(int index);

    template<typename T, typename... Args>
    T* newElement(Args&&... args);

    template<typename T>
    void deleteElement(T* ptr);

private:
    void* useMemory(size_t size);
    void freeMemory(void* ptr, size_t size);

private:
    HashBucket();
};

#include <MemoryPool.tcc>

#endif