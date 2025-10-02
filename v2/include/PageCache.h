#pragma once 
#include <Common.hpp>

class PageCache {
public:
    static PageCache& GetInstance() {
        return _sInt;
    }

    Span* NewSpan(size_t kpage); // 拿出大小为k页的span

    std::mutex& GetMutex();

private:
    std::array<SpanList, PAGE_NUM> _spanLists;
    std::mutex _pageMtx; // pc整体锁

private:
    PageCache() {};
    PageCache(const PageCache&) = delete;
    PageCache& operator=(const PageCache&) = delete;
    static PageCache _sInt;
};