#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <mutex>
#include <assert.h>

typedef size_t PageID;

using std::vector;
using std::cout;
using std::endl;
using std::array;

static const size_t FREE_LIST_NUM = 208;
static const size_t MAX_BYTES = 256 * 1024;
static const size_t PAGE_NUM = 129;
static const size_t PAGE_SHIFT = 13; // 定义页大小8KB

// #ifdef _WIN64
//     typedef unsigned long long PageID; 
// #elif _WIN32
//     typedef size_t PageID;
// #else // Linux
//     typedef size_t PageID;
// #endif

#ifdef _WIN32 // Windows下的系统调用接口
    #include <Windows.h>
#else 
    #include <unistd.h> // brk, sbrk
    #include <sys/mman.h> // mmap, munmap
#endif

inline static void* SystemAlloc(size_t kpage) { // 每页按2 ^ 13 = 8KB
#ifdef _WIN32
    void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
    void* ptr;
    if((kpage << PAGE_SHIFT) < 128 * 1024) {
        cout << "sbrk():" << endl;
        void* initial_brk = sbrk(0);
        cout << "Initial break: " << initial_brk << endl; 
        ptr = sbrk(kpage << PAGE_SHIFT); // kpage * 8KB
        void* current_brk = sbrk(0);
        cout << "Current break: " << current_brk << endl; 
        if(ptr == nullptr) throw std::bad_alloc();
    } else {
        cout << "mmap():" << endl;
        ptr = mmap64(NULL, kpage << PAGE_SHIFT, PROT_READ | PROT_WRITE, 
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(ptr == MAP_FAILED) {
            perror("mmap failed");
            throw std::bad_alloc();
        }
    }
#endif
    cout << "SystemAlloc : " << (kpage << PAGE_SHIFT) << endl;
    return ptr;
}


// 如果没有引用，返回的是一个右值，
// 因为ObjNext返回值是一个拷贝，是一个临时对象，
// 而临时对象具有常属性，不能被修改，也就是一个右值，
// 右值无法进行赋值操作
static void*& ObjNext(void* obj) { // obj的头4/8字节
    return *(void**)obj;
}

class FreeList {
public:
    bool Empty() {
        return _freeList == nullptr;
    }

    void Push(void* obj) { // 回收空间
        assert(obj);

        ObjNext(obj) = _freeList;
        _freeList = obj;
        ++_size;
    }

    void* Pop() { // 提供空间
        assert(_freeList);

        void* obj = _freeList;
        _freeList = ObjNext(obj);
        --_size;
        return obj;
    }

    size_t& MaxSize() { // FreeList未到上限时，能够申请的最大块空间
        return _maxSize;
    }

    void PushRange(void* start, void* end, size_t n) {
        ObjNext(end) = nullptr;
        _freeList = start;

        _size += n;
    }

    size_t Size() {
        return _size;
    }

    // [start, end] 闭区间
    void PopRange(void*& start, void*& end, size_t n) {
        assert(n > 0);
        assert(n <= _size);

        start = end =  _freeList;
        for(int i = 0; i < n - 1; ++i) {
            end = ObjNext(end);
        }
        _freeList = ObjNext(end);
        ObjNext(end) = nullptr;
        _size -= n;
    }

private:
    void* _freeList = nullptr;
    size_t _maxSize = 1;
    size_t _size = 0; // 当前_freeList中有多少块空间（用来判断是否需要归还给cc）
};

struct Span {
public:
    PageID _pageID = 0; // 页号
    size_t _n = 0; // 当前span管理的页的数量

    void* _freeList = nullptr; // 每个span下面挂的小块空间的头节点
    size_t use_count = 0; // 当前span分配出去多少个块空间

    Span* _prev = nullptr;
    Span* _next = nullptr;
};

// 双向循环链表
class SpanList {
public:
    SpanList() {
        _head = new Span;

        _head->_prev = _head;
        _head->_next = _head;
    }

    void Insert(Span* pos, Span* ptr) {
        // 在pos前面插入ptr
        assert(pos);
        assert(ptr);

        pos->_prev->_next = ptr;
        ptr->_prev = pos->_prev;

        ptr->_next = pos;
        pos->_prev = ptr;
    }

    void Erase(Span* pos) {
        assert(pos);
        assert(pos != _head);

        pos->_prev->_next = pos->_next;
        pos->_next->_prev = pos->_prev;

        // to do: 回收
    }

    Span* Begin() {
        return _head->_next;
    }

    Span* End() {
        return _head;
    }

    void PushFront(Span* span) {
        Insert(Begin(), span);
    }

    bool Empty() {
        return Begin() == End();
    }

    Span* PopFront() {
        Span* front = Begin();
        Erase(front);
        return front;
    }

private:
    Span* _head; // 哨兵头节点

public:
    std::mutex _mtx; // 每个cc中的哈希桶（Span链表）都需要一个桶锁
};