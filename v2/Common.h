#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <assert.h>
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

private:
    void* _freeList = nullptr;
};