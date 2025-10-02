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

// A fixed-length memory pool for specific objects

#pragma once
#include "Common.hpp"

// template<size_t N> 定长内存池可以采用非类型模板参数
template<class T> // 这里采用类型模板参数，方便针对不同的类生成定长内存池
class ObjectPool {
public:
    T* New() {
        T* obj = nullptr;
        if(_freelist != nullptr) {
            void* next = *(void**)_freelist; // 取下一个内存块地址
            obj = (T*)_freelist;
            _freelist = next;
        } else {
            if(_remanentBytes < sizeof(T)) {
                _remanentBytes = 128 * 1024;
                // _memory = (char*)malloc(_remanentBytes);
                _memory = (char*)SystemAlloc(_remanentBytes >> 13); // 申请16页
                if (_memory == nullptr) throw std::bad_alloc();
            }
            obj = (T*)_memory;
            size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
            _memory += objSize;
            _remanentBytes -= objSize;
        }
        new(obj) T; // 通过placement new调用T的构造函数初始化

        return obj;
    } 

    void Delete(T* obj) {
        // 显示调用析构函数
        obj->~T();

        // 头插
        // 用一个二级指针强转obj后再解引用，这样就会得到一个指针大小的空间(直接取决于平台32位/64位)
        *(void**)obj = _freelist; // 新块指向旧块（或空）
        _freelist = obj; // 头指针指向新块
    }

private:
    char* _memory = nullptr; // void* 没办法/很难执行+=，解引用等操作
    size_t _remanentBytes = 0;
    void* _freelist = nullptr;
};