#include "ThreadPool.h"
#include <iostream>
#include <functional>

int main() {
    int n;  //The work-thread's number
    std::cin >> n;
    ThreadPool threadPool(n);

    for(int i = 1; i <= n; i++) {
        std::function<void()> task = ([i] {
            std::string out = "Thread ";
            out += std::to_string(i);
            out += " has worked successfully!\n";
            std::cout << out;
        });

        threadPool.SubmitTask(task);
    }

    // 主线程执行其他操作
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}