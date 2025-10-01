#include "CentralCache.h"

CentralCache CentralCache::_sInst; // cc的饿汉对象，不能写在.h中，防止头文件被包含多次，导致链接错误

size_t CentralCache::FetchRangeObj(void *&start, void *&end, size_t batchNum, size_t size)
{
    size_t index = SizeClass::Index(size);

    std::lock_guard<std::mutex> lgmtx(_spanLists.at(index)._mtx);

    // 获取一个非空的span
    Span* span = GetOneSpan(_spanLists.at(index), size);
    assert(span);
    assert(span->_freeList);
    
    // 
    start = end = span->_freeList;
    size_t actualNum = 1;
    size_t i = 0;
    // 尽可能满足batchNum的获取数，不够则退出
    while(i < batchNum - 1 && ObjNext(end) != nullptr) {
        end = ObjNext(end);
        ++i;
        ++actualNum;
    }
    // ObjNext(end)可能为nullptr，可能为剩余空间的头节点，更新对应桶的freeList
    span->_freeList = ObjNext(end);
    ObjNext(end) = nullptr;

    return actualNum;
}

Span* CentralCache::GetOneSpan(SpanList &list, size_t size)
{
    return nullptr;
}
