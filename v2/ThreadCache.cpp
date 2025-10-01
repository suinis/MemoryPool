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

void ThreadCache::Deallocate(void *obj, size_t size)
{
    assert(obj);
    assert(size <= MAX_BYTES);

    size_t index = SizeClass::Index(size);
    _freeLists.at(index).Push(obj);
}

void* ThreadCache::FetchFromCentralCache(size_t index, size_t alignSize)
{
    // MaxSize：index对应空闲链表当次能提供给tc alignSize大小空间的个数
    // NumMoveSize：tc单次能够向cc申请的alignSize大小空间的最大个数
    // alignSize = 8B，MaxSize = 1时，NumMoveSize=512，取最小个数1
    size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(alignSize));

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
    _freeLists[index].PushRange(ObjNext(start), end);
    return start;
}
