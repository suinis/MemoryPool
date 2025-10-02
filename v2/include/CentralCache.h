#pragma once
#include "Common.hpp"
#include "ThreadCache.h"

// 饿汉单例
class CentralCache {
public:
    static CentralCache& GetInstance() {
        return _sInst;
    }

    size_t FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t alignSize);

    Span* GetOneSpan(SpanList& list, size_t size);

    // 将tc归还的多块空间放入span
    void ReleaseListToSpans(void* start, size_t alignSize);

private:
    CentralCache() {};
    CentralCache(const CentralCache&) = delete;
    CentralCache& operator=(const CentralCache&) = delete;

    std::array<SpanList, FREE_LIST_NUM> _spanLists;
    static CentralCache _sInst; // 饿汉单例，静态成员属性：类内申明、类外初始化
};