#include "TasksQueue.h"
#include "ThreadPool.h"
#include <iostream>

int main() {
    // 1. 测试非标准异常
    pool.SubmitTask([] { throw "原始字符串异常"; });

    // 2. 测试内存耗尽场景
    pool.SubmitTask([] {
        std::vector<int> vec;
        while (true)
            vec.push_back(0); // 触发std::bad_alloc
    });

    // 3. 测试析构安全性
    // 两个花括号能够形成该代码块的作用域，在花括号结束处对象调用析构函数释放资源，提升测试的可观察性和可控性
    {
        ThreadPool temp_pool(2);
        temp_pool.SubmitTask([] { std::this_thread::sleep_for(1s); });
    } // 离开作用域时测试析构是否正常

    // 主线程执行其他操作
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}