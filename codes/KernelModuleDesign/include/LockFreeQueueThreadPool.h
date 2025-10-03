#pragma once

#include "LockFreeQueue.h"
#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <vector>

class LockFreeQueueThreadPool {
  private:
    LockFreeQueue<std::function<void()>> taskQueue;
    std::vector<std::thread> workers;   // 可复用工作线程（消费者线程）
    std::atomic<bool> isRunning{false}; // 线程池运行状态

    void start_threads(size_t thread_num) {
        isRunning.store(true);

        // 创建指定数量的工作线程
        for (size_t i = 0; i < thread_num; i++) {
            workers.emplace_back(&LockFreeQueueThreadPool::worker_thread, this);
        }
    }

    void stop_threads() {
        isRunning.store(false);
        // 等待所有工作线程退出
        for (auto& worker : workers) {
            worker.join();
        }
    }

    // 实现工作线程组
    void worker_thread() {
        while (isRunning.load()) {
            std::function<void()> task;
            if (taskQueue.Pop(task)) {
                try {
                    task();
                } catch (const std::exception& e) {
                    // 捕获任务执行中的异常
                    std::cerr << "Exception in worker thread: " << e.what() << std::endl;
                } catch (...) {
                    // 处理其他未知异常
                    std::cerr << "Unknown exception in worker thread" << std::endl;
                }
            } else {
                // 队列为空时，让出CPU
                std::this_thread::yield();
            }
        }
    }

  public:
    // 构造函数
    explicit LockFreeQueueThreadPool(size_t thread_num = std::thread::hardware_concurrency()) {
        if (thread_num == 0) {
            throw std::invalid_argument("thread_num must be greater than 0");
        }

        // 启动工作线程
        start_threads(thread_num);
    }

    // 析构函数
    ~LockFreeQueueThreadPool() { stop_threads(); };

    // 禁止拷贝构造和赋值
    LockFreeQueueThreadPool(const LockFreeQueueThreadPool&) = delete;
    LockFreeQueueThreadPool& operator=(const LockFreeQueueThreadPool&) = delete;

    /*
    * 函数目的：提交任务并返回future
    * 语法说明
        &&：表示万能引用，用于接受任意类型的参数，包括函数、lambda表达式等
        Args&&... args：接收任务的参数（...表示可变参数，支持任意数量和类型的参数）
        ->：返回一个future对象，其类型由任务的返回值类型决定
        using：定义别名
        packaged_task：要求任务无参数
            若f是int add(int a, int b)，args是3和5，则bind后得到一个无参对象，调用它时会执行add(3,5)
        std::bind(...)：绑定任务函数f和参数args，返回一个可调用对象
        std::forward：完美转发参数，确保参数的左值右值属性不被改变，避免不必要的拷贝
        std::make_shared：packaged_task不能拷贝（只能移动），用shared_ptr可以安全管理生命周期
    */
    template <typename F, typename... Args> // ...：表示可变参数
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        // 确定任务返回类型
        using return_type = typename std::result_of<F(Args...)>::type;

        // 使用package_task包装任务
        // 即使 shared_ptr 引用计数归 0，packaged_task 也不会立即被销毁，因为 future 会通过内部机制 “延长” 关键数据的生命周期
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        // 获取future用于获取任务返回值
        std::future<return_type> result = task->get_future();

        /*
        用 lambda
        表达式包装packaged_task，形成一个std::function<void()>类型的任务（无参无返回值，符合队列中任务的类型）。
        [task]：捕获shared_ptr<packaged_task>，确保任务在队列中时packaged_task不会被销毁。
        (*task)()：调用packaged_task（即执行原始任务f(args...)），执行完成后结果会自动同步到future
        */
        taskQueue.Push([task]() { (*task)(); });

        // 返回future对象
        return result;
    }

    // 获取线程数量
    size_t get_thread_count() const { return workers.size(); }

    // 检查线程池是否处于运行态
    bool is_running() const { return isRunning.load(); }
};
