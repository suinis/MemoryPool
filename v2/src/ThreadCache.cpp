#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::Allocate(size_t size) {
    assert(size <= MAX_BYTES);

    size_t alignSize = SizeClass::RoundUp(size);
    size_t index = SizeClass::Index(size);
    
    if(!_freeLists.at(index).Empty()) {
        return _freeLists.at(index).Pop();
    } 
    return FetchFromCentralCache(index, alignSize);
}

void ThreadCache::Deallocate(void *obj, size_t alignSize)
{
    assert(obj);
    assert(alignSize <= MAX_BYTES);

    size_t index = SizeClass::Index(alignSize);
    _freeLists.at(index).Push(obj);

    // 当前桶中块数 >= 单次批量申请块数的时候归还空间
    // to do：这里是不是可以不必每次deallocate都判断一次并做回收
    FreeList& cutFreeList = _freeLists.at(index);
    if(cutFreeList.Size() >= cutFreeList.MaxSize()) {
        ListTooLong(cutFreeList, alignSize);
    }
}

void* ThreadCache::FetchFromCentralCache(size_t index, size_t alignSize)
{
    
// <Windows.h>中也有min且是宏定义，主要是没用命名空间导致的命名空间污染，这里条件编译简单解决一下
#ifdef _WIN32 
    size_t batchNum = min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(alignSize));
#else
    // MaxSize：index对应空闲链表当次能提供给tc alignSize大小空间的个数
    // NumMoveSize：tc单次能够向cc申请的alignSize大小空间的最大个数
    // alignSize = 8B，MaxSize = 1时，NumMoveSize=512，取最小个数1
    size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(alignSize));
#endif // WIN32

    if(batchNum == _freeLists[index].MaxSize()) {
        ++(_freeLists[index].MaxSize());
    }

    void* start = nullptr;
    void* end = nullptr;

    // 实际获取到的alignSize大小空间的个数
    size_t actualNum = CentralCache::GetInstance().FetchRangeObj(start, end, batchNum, alignSize);
    assert(actualNum >= 1);

    if(actualNum == 1) {
        assert(start == end);
        return start;
    }
    // actualNum > 1，则返回头一个，剩下的插入该tc对应index空闲链表
    _freeLists[index].PushRange(ObjNext(start), end, actualNum - 1);
    return start;
}

void ThreadCache::ListTooLong(FreeList &list, size_t alignSize)
{
    void* start = nullptr;
    void* end = nullptr;

    list.PopRange(start, end, list.MaxSize());
    // to do: mutex
    CentralCache::GetInstance().ReleaseListToSpans(start, alignSize);
}
