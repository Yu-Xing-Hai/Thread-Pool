#pragma once

#include "LockFreeQueue.h"
#include <functional>

class ThreadPool_LockFreeQueue {
private:
    LockFreeQueue<std::functional<void()>> LockFreeQueue;
    std::vector<std::thread> workers; // 可复用工作线程（消费者线程）

public:
    ThreadPool_LockFreeQueue(size_t thread_num) {
        for(size_t i = 0; i < thread_num; i++) {
            workers.emplace_back([this] {
                
            });
        }
    }
}