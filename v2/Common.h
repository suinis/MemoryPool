#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <mutex>
#include <assert.h>

// #ifdef _WIN64
//     typedef unsigned long long PageID; 
// #elif _WIN32
//     typedef size_t PageID;
// #else // Linux
//     typedef size_t PageID;
// #endif
typedef size_t PageID;

using std::vector;
using std::cout;
using std::endl;
using std::array;

static const size_t FREE_LIST_NUM = 208;
static const size_t MAX_BYTES = 256 * 1024;

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
    }

    void* Pop() { // 提供空间
        assert(_freeList);

        void* obj = _freeList;
        _freeList = ObjNext(obj);
        return obj;
    }

    size_t& MaxSize() { // FreeList未到上限时，能够申请的最大块空间
        return _maxSize;
    }

    void PushRange(void* start, void* end) {
        ObjNext(end) = nullptr;
        _freeList = start;
    }

private:
    void* _freeList = nullptr;
    size_t _maxSize = 1;
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

private:
    Span* _head; // 哨兵头节点

public:
    std::mutex _mtx; // 每个cc中的哈希桶（Span链表）都需要一个桶锁
};