// #ifndef INCLUDE_WORKTHREADSGROUP_H
// #define INCLUDE_WORKTHREADSGROUP_H
#pragma once // 编译器特性，效果相同但无需手动定义宏

/*
【第一版】线程池类实现
当前目标：
1. 实现工作线程组
2. 实现添加任务函数
3. 实现工作线程创建

后续规划：

*/
#include "TasksQueue.h"
#include <condition_variable> //线程同步工具，用于阻塞或唤醒线程
#include <functional>
#include <iostream>
#include <mutex> //互斥锁的库
#include <queue>
#include <thread>
#include <atomic>

class ThreadPool {
  private:
    TasksQueue tasks_queue;             // 实例化任务队列
    std::vector<std::thread> workers;   // 可复用工作线程
    std::atomic<bool> stop_flag; // 创建原子布尔值，不可直接初始化

  public:
    /*构造函数*/
    /*
        explicit：强制要求代码必须显式调用构造函数,
        size_t：无符号整数类型，用于表示对象大小或数组索引
    */
    explicit ThreadPool(size_t thread_num) : stop_flag(false) {  //在构造函数中初始化stop_flag
        for (size_t i = 0; i < thread_num; i++) {
            workers.emplace_back([this] {
                while (!stop_flag.load()) {
                    auto task = tasks_queue.Pop();
                    if (task)
                        task();
                }
            });
        }
    }

	/*提交任务*/
	void SubmitTask(std::function<void()> task) {
		tasks_queue.Push(std::move(task));
	}

    ~ThreadPool() {
		stop_flag.store(true);
		tasks_queue.Stop();  //唤醒所有线程
		for(auto& worker : workers) {
			if(worker.joinable())
				worker.join();  //阻塞主线程，释放worker线程占有的资源，再进行析构便不会造成内存泄漏
		}
	}
};

// #endif