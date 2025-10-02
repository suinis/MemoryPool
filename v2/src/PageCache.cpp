#include "PageCache.h"

PageCache PageCache::_sInt;

Span *PageCache::NewSpan(size_t kpage)
{
    assert(kpage > 0 && kpage < PAGE_NUM);
    // pc对应npage桶有span
    if(!_spanLists.at(kpage).Empty()) {
        return _spanLists.at(kpage).PopFront();
    }
    // npage桶没有，更大页桶有
    for(int i = kpage + 1; i < PAGE_NUM; ++i) {
        if(!_spanLists.at(i).Empty()) {
            // 拿出nSpan，此时已经进行了pop
            Span* nSpan = _spanLists.at(i).PopFront();

            Span* kSpan = new Span;
            kSpan->_pageID = nSpan->_pageID;
            kSpan->_n = kpage;

            nSpan->_pageID += kpage;
            nSpan->_n -= kpage;

            cout << "kpageID: " << kSpan->_pageID << ", k_n: " << kSpan->_n << endl;
            cout << "npageID: " << nSpan->_pageID << ", n_n: " << nSpan->_n << endl; 

            // 剩余放回pc
            _spanLists.at(nSpan->_n).PushFront(nSpan);
            // 切割的kpage返回给cc
            return kSpan;
        }
    }
    // 更大页桶也没有，向系统申请
    void* ptr = SystemAlloc(PAGE_NUM - 1);

    Span* bigSpan = new Span;
    // 参考输出理解，第一次获取的ptr随机
    // 由于获取的内存有连续性的特点，故再次申请得到的ptr应该是紧跟128页之后的地址
    bigSpan->_pageID = ((PageID)ptr) >> PAGE_SHIFT;
    bigSpan->_n = PAGE_NUM - 1;
    cout << "ptr: " << ptr << ", (PageID)ptr: " << (PageID)ptr << endl;
    cout << "pageID: " << bigSpan->_pageID << endl;
    // 插入到pc的最后一个桶
    _spanLists.at(PAGE_NUM - 1).PushFront(bigSpan);

    // 重新分配
    return NewSpan(kpage);
}

std::mutex &PageCache::GetMutex()
{
    return _pageMtx;
}
