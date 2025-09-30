// #ifndef INCLUDE_WORKTHREADSGROUP_H
// #define INCLUDE_WORKTHREADSGROUP_H
#pragma once // 编译器特性，效果相同但无需手动定义宏

/*
【第一版】线程池类实现
当前目标：
1. 实现工作线程组
2. 实现添加任务函数
3. 实现工作线程创建

【第二版】线程池类实现
当前目标：
1. 实现异常处理(初级)【√】

*/
#include "TasksQueue.h"
#include <atomic>
#include <condition_variable> //线程同步工具，用于阻塞或唤醒线程
#include <functional>
#include <iostream>
#include <mutex> //互斥锁的库
#include <queue>
#include <thread>

class ThreadPool {
  private:
    TasksQueue tasks_queue;           // 实例化任务队列
    std::vector<std::thread> workers; // 可复用工作线程（消费者线程）
    std::atomic<bool> stop_flag;      // 创建原子布尔值，不可直接初始化

  public:
    /*构造函数*/
    /*
        explicit：强制要求代码必须显式调用构造函数,
        size_t：无符号整数类型，用于表示对象大小或数组索引
    */
    explicit ThreadPool(size_t thread_num) : stop_flag(false) { // 在构造函数中初始化stop_flag
        for (size_t i = 0; i < thread_num; i++) {  // 构造用户指定数量的工作线程
            try {
                workers.emplace_back([this] {
                    while (!stop_flag.load()) {
                        auto task = tasks_queue.Pop();
                        if (task) {
                            try {  //任务执行异常时的异常处理
                                task();
                            } catch (const std::exception& e) {  //`const`防止修改异常对象，`&`可避免拷贝，提升效率
                                std::cerr<< "Task failed:" << e.what() << std::endl;  //cerr比起cout来说：无缓存、立即输出、程序奔溃也可输出，因此常用于异常错误输出
                            } catch (...) {  //全捕获分支，避免非标准异常
                                std::cerr << "Unknow error" << std::endl;
                            }
                        }
                        else
                            continue;  //预防空任务
                    }
                });
            } catch (const std::exception& e) {
                std::cerr << "Fail to create thread:" << e.what() << std::endl;
                tasks_queue.Stop();  //清理已创建的线程
                throw;
            }
        }
    }

    /*提交任务*/
    void SubmitTask(std::function<void()> task) { 
        try {  //处理Push函数返回的异常
            tasks_queue.Push(std::move(task)); 
        } catch (const std::exception& e) {
            std::cerr << "Fail to push:" << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Fail to push: Unknow error" << std::endl;
        }
    }

    ~ThreadPool() {
        stop_flag.store(true);
        tasks_queue.Stop(); // 唤醒所有线程
        for (auto& worker : workers) {
            try {
                if (worker.joinable())
                worker.join(); // 先阻塞主线程，待释放完worker线程占有的资源，再进行析构便不会造成内存泄漏
            } catch (const std::exception& e) {
                std::cerr << "Thread merge error:" << e.what() << std::endl;
            }
        }
    }
};

// #endif