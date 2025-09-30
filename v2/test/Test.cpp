#include "ObjectPool.h"
#include "ThreadCache.h"
#include "ConcurrentAlloc.h"

struct TreeNode {
    int _val;
    TreeNode *_left;
    TreeNode *_right;

    TreeNode() : _val(0), _left(nullptr), _right(nullptr) {}
};

// malloc和当前定长内存池性能对比
void TestObjectPool() {
    // 申请释放的轮次
    const size_t Rounds = 10;

    // 每轮申请释放多少次
    const size_t N = 10000;

    std::vector<TreeNode*> v1;
    v1.reserve(N);

    size_t begin1 = clock();
    for(int j = 0; j < Rounds; ++j) {
        for(int i = 0; i < N; ++i) {
            v1.push_back(new TreeNode); // 底层调用的是malloc
        }
        for(int i = 0; i < N; ++i) {
            delete v1[i];
        }
        v1.clear();
    }
    size_t end1 = clock();

    std::vector<TreeNode*> v2;
    v2.reserve(N);

    ObjectPool<TreeNode> TNPool;
    size_t begin2 = clock();
    for(int j = 0; j < Rounds; ++j) {
        for(int i = 0; i < N; ++i) {
            v2.push_back(TNPool.New());
        }
        for(int i = 0; i < N; ++i) {
            TNPool.Delete(v2[i]);
        }
        v2.clear();
    }
    size_t end2 = clock();
    cout << "new delete cost timme: " << end1 - begin1 << endl;
    cout << "ObjectPool New Delete cost time: " << end2 - begin2 << endl; 
}

void AllocTest() {
    auto allocfunc = [](int size){
        for(int i = 0; i < 5; ++i) {
            ConcurrentAlloc(size);
        }
    };
    std::thread th1(allocfunc, 6);
    std::thread th2(allocfunc, 7);

    th1.join();
    th2.join();
}

void UintTest() {
    TestObjectPool();
    cout << " ====================================" << endl;

    AllocTest();
    cout << " ====================================" << endl;
}

int main() {
    // for valgrind test:
    // valgrind --leak-check=full --show-leak-kinds=all --log-file=TestObjectPool_g ./Test
    UintTest();
}