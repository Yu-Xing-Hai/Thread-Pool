#include "TasksQueue.h"
#include "ThreadPool.h"
#include <iostream>

int main() {
    ThreadPool pool(2);  //创建两个可复用的消费者线程
    std::cout << "Task1 executed by thread" << std::this_thread::get_id() << std::endl;

    //生产者提交任务
    pool.SubmitTask([] {
        std::cout << "Task1 executed by thread" << std::this_thread::get_id() << std::endl;
    });
    pool.SubmitTask([] {
        std::cout << "Task1 executed by thread" << std::this_thread::get_id() << std::endl;
    });
    

    //主线程执行其他操作
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}