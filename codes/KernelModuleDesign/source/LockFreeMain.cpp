#include <iostream>
#include <chrono>
#include "LockFreeQueueThreadPool.h"

// 示例任务：计算平方
int square(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟耗时操作
    return x * x;
}

// 示例任务：无返回值
void print_message(const std::string& msg) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "Message: " << msg << std::endl;
}

int main() {
    try {
        // 创建线程池，默认使用CPU核心数的线程
        LockFreeQueueThreadPool pool;
        std::cout << "ThreadPool created, thread count: " << pool.get_thread_count() << std::endl;
        
        // 提交多个任务
        auto f1 = pool.submit(square, 10);
        auto f2 = pool.submit(square, 20);
        auto f3 = pool.submit(print_message, "Task is running...");
        auto f4 = pool.submit([]() { 
            return "Anonymous task completed"; 
        });
        
        // 获取任务结果
        std::cout << "The square of 10 is: " << f1.get() << std::endl;
        std::cout << "The square of 20 is: " << f2.get() << std::endl;
        
        // 等待无返回值的任务完成
        f3.wait();
        
        // 获取匿名任务结果
        std::cout << f4.get() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}