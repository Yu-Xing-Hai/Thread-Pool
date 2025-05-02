#include "ErrorTest.h"
#include "StressTest.h"
#include "TasksQueue.h"
#include "ThreadPool.h"
#include <iostream>

int main() {
    // 主线程执行其他操作
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}