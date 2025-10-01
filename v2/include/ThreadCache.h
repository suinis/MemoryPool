#pragma once
#include "Common.h"

class ThreadCache {
public:
    void* Allocate(size_t size); // 线程申请size大小的空间
    void Deallocate(void* obj, size_t size); // 回收线程中大小为size的obj空间

    void* FetchFromCentralCache(size_t index, size_t alignSize);

private:
    std::array<FreeList, FREE_LIST_NUM> _freeLists;
    // FreeList _freeLists[FREE_LIST_NUM];
};

// TLS 线程本地存储
static thread_local ThreadCache* pTLSThreadCache = nullptr;

class SizeClass {
public:
    static size_t RoundUp(size_t size)
    {
        assert(size > 0);
        if(size <= _endSize[0]) {
            return _RoundUp(size, _alignNum[0]);
        } else if(size <= _endSize[1]) {
            return _RoundUp(size, _alignNum[1]);
        } else if(size <= _endSize[2]) {
            return _RoundUp(size, _alignNum[2]);
        } else if(size <= _endSize[3]) {
            return _RoundUp(size, _alignNum[3]);
        } else if(size <= _endSize[4]) {
            return _RoundUp(size, _alignNum[4]);
        } else {
            assert(false);
            return -1;
        }
    }

    static size_t _RoundUp(size_t size, size_t alignNum) {
        // 位运算
        // return (size + alignNum - 1) & ~(alignNum - 1);
        return ((size + alignNum - 1) / alignNum) * alignNum;
    }

    static size_t Index(size_t size) {
        assert(size > 0);
        if(size <= _endSize[0]) {
            return (size - 1) / _alignNum[0];
        } else if(size <= _endSize[1]) {
            return (size - _listSizePerGroup[0] - 1) / _alignNum[1];
        } else if(size <= _endSize[2]) {
            return (size - _listSizePerGroup[0] - _listSizePerGroup[1] - 1) / _alignNum[2];
        } else if(size <= _endSize[3]) {
            return (size - _listSizePerGroup[0] - _listSizePerGroup[1]- _listSizePerGroup[2] - 1) / _alignNum[3];
        } else if(size <= _endSize[4]) {
            return (size - _listSizePerGroup[0] - _listSizePerGroup[1]- _listSizePerGroup[2] - _listSizePerGroup[3] - 1) / _alignNum[4];
        } else {
            assert(false);
            return -1;
        }
    }

    static size_t NumMoveSize(size_t size) {
        assert(size > 0);

        int num = MAX_BYTES / size;
        if(num > 512) {
            // 当size = 8B时，num = 32K，量太多容易造成空间浪费，限制小空间申请的量
            num = 512;
        } else if(num < 2) {
            // 当size = 256KB时，num = 1，如果一下需要1MB，则定为2可以减少调用次数
            num = 2;
        }

        // 小对象一次批量上限高
        // 大对象一次批量上限低
        return num;
    }

private:
    // 静态常量类内定义（需要C17，要么类内声明类外定义，否则会报链接错误）
    static constexpr array<size_t, 5> _alignNum = {8, 16, 128, 1024, 8 * 1024};
    static constexpr array<size_t, 5> _listSizePerGroup = {16, 56, 56, 56, 24};
    static constexpr array<size_t, 5> _endSize = {128, 1024, 8 * 1024, 64 * 1024, 256 * 1024};
};