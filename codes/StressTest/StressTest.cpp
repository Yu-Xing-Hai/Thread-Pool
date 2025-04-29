/*
压力测试代码说明

*/
void StressTest() {
    /*测试初始化*/
    ThreadPool pool(std::thread::hardware_concurrency()); // 创建线程池，线程数等于CPU核心数

    /*原子计数器*/
    std::atomic<int> counter{0}; // 线程安全的计数器

    /*任务提交风暴*/
    for (int i = 0; i < 100000; ++i) {
        pool.SubmitTask([&counter] { counter++; });
    }

    while (counter < 100000)
        std::this_thread::yield();  //主线程让出CPU时间片，避免忙等待消耗CPU
    std::cout << "压力测试通过" << std::endl;
    std::cout << counter.load() << std::endl;
}