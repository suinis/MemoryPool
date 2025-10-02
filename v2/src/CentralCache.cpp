#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst; // cc的饿汉对象，不能写在.h中，防止头文件被包含多次，导致链接错误

size_t CentralCache::FetchRangeObj(void *&start, void *&end, size_t batchNum, size_t alignSize)
{
    size_t index = SizeClass::Index(alignSize);

_spanLists.at(index)._mtx.lock();

    // 获取一个非空的span
    Span* span = GetOneSpan(_spanLists.at(index), alignSize);
    assert(span);
    assert(span->_freeList);
    
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
    span->use_count += actualNum;
    ObjNext(end) = nullptr;

_spanLists.at(index)._mtx.unlock();
    return actualNum;
}

// 获取一个管理空间非空的span
Span* CentralCache::GetOneSpan(SpanList &list, size_t alignSize)
{
    // 先在cc中找管理空间非空的span
    Span* it  = list.Begin();
    while(it != list.End()) {
        if(it->_freeList != nullptr) { // 对应span下有非空空间
            return it;
        } 
        it = it->_next;
    }

    // cc没找到管理空间非空的span
    /* 从这里开始向pc申请npage span，故这里可以先解cc对应桶的锁，
    让其他线程可以操作该桶（归还span） */
list._mtx.unlock();
    // 将size转换为匹配的页数，以从pc获取合适的span
    size_t pageNum = SizeClass::NumMovePage(alignSize);

    // 从pc获取未划分过的span
PageCache::GetInstance().GetMutex().lock();
    Span* span = PageCache::GetInstance().NewSpan(pageNum);
PageCache::GetInstance().GetMutex().unlock();
    
    // 页号 * 页大小 = span所管理空间的首地址
    char* start = (char*)(span->_pageID << PAGE_SHIFT);
    char* end = (char*)(start + (span->_n << PAGE_SHIFT));
    // 先放入span的_freeList中（此时还未划分）
    span->_freeList = start;
    // 划分
    void* tail = start;
    start += alignSize;
    while (start < end) {
        ObjNext(tail) = start;
        tail = start;
        start += alignSize;
    }
    ObjNext(tail) = nullptr;

    // 改变的是pc获取到的span，还需要链接到cc对应SpanList上
list._mtx.lock();
    list.PushFront(span);

    return span;
}

void CentralCache::ReleaseListToSpans(void *start, size_t alignSize)
{
    
}
