# 一、学习目标​​
- 掌握现代C++核心特性：std::thread、std::mutex、std::future
- 理解生产者-消费者模型（工业控制核心模式）
- 培养资源管理意识（避免内存泄漏）
# 二、分步指南
1. **基础版本**
   - 用`std::queue`实现任务队列【√】
   - 通过`std::condition_variable`实现任务调度【√】
   - 支持固定工作线程数（如4个worker线程）【√】
   - 完善异常处理机制
     - 标准、非标准异常的捕获【√】
       - 在标准try-catch后添加全捕获分支
     - 任务队列中的异常传播【√】
       - 如Push或Pop过程中出现异常
     - 析构函数中的异常处理【√】
       - worker.join()时若任务抛出异常，会导致析构函数传播异常
     - 资源泄露风险【√】
   - 支持任务返回值【√】
     - 使用`<future>`库完成异步任务结果的获取
   - 日志记录的可配置性

2. **进阶优化**
   - 实现无锁队列【√】
   - 实现动态扩容：根据队列负载自动增减线程
   - 集成性能监控：统计任务平均耗时/队列积压量  
   - 添加优雅退出机制：处理未完成任务【√】
   - 实现更优雅的关闭
     - 允许等待队列任务完成

3. **工程化扩展** 
   - 编写单元测试（Google Test验证线程安全）  
   - 输出性能对比报告（对比直接创建线程的差异）  
# 三、线程池的核心原理
> 线程池是一种多线程资源管理技术，通过创建一组线程并复用，减少频繁创建/销毁线程的开销
- 核心目标
  - **线程复用**：避免线程重复创建、降低系统资源消耗
  - **任务队列管理**：通过缓冲队列调度任务，防止任务堆积导致内存溢出
  - **并发控制**：限制同时活跃的线程数，防止资源竞争和上下文切换过载
# 四、线程池的核心模块
## 4.1  任务队列【核心数据结构】
- 功能
  - 存储待执行任务，避免频繁创建线程，保持线程复用，通常使用线程安全的队列(如std::queue或者优先队列)
- 实现要点
  - 使用模板template支持多种任务类型
  - 结合互斥锁和条件变量实现线程安全操作
## 4.2  工作线程组【狭义上的线程池】
- 功能
  - **预创建的线程**循环执行队列中的任务
- 实现要点
  - 线程在空闲时通过条件变量阻塞，等待任务唤醒
## 4.3  同步机制
- 互斥锁mutex
- 条件变量Condition_virable
- 原子变量atomic
  - 用于管理线程池状态，如`stop_flag`可被用于控制工作线程组是否停止工作
# 五、测试结果汇总
- 对照成熟线程池：BS：：threadpool
  - github：https://github.com/bshoshany/thread-pool.git
- 对照测试结果
```TEXT
#*********************基于queue的线程池测试结果*********************
$ ./benchmark_compare.exe
Starting thread pool performance comparison test...

=== Light task test ===
Number of tasks: 1000, Complexity: 100
BS::thread_pool: 2255 us (2.255 ms) (Result: 138460)
My thread pool:    2763 us (2.763 ms) (Result: 138460)

=== Medium task test ===
Number of tasks: 100, Complexity: 1000
BS::thread_pool: 490 us (0.49 ms) (Result: 171876)
My thread pool:    483 us (0.483 ms) (Result: 171876)

=== Heavy task test ===
Number of tasks: 10, Complexity: 10000
BS::thread_pool: 557 us (0.557 ms) (Result: 142441)
My thread pool:    588 us (0.588 ms) (Result: 142441)

Test completed!
```

```TEXT
#*********************基于无锁队列的线程池测试结果*********************
Starting thread pool performance comparison test...

=== Light task test ===
Number of tasks: 1000, Complexity: 100
BS::thread_pool: 2898 us (2.898 ms) (Result: 138460)
My thread pool:    992 us (0.992 ms) (Result: 138460)

=== Medium task test ===
Number of tasks: 100, Complexity: 1000
BS::thread_pool: 661 us (0.661 ms) (Result: 171876)
My thread pool:    698 us (0.698 ms) (Result: 171876)

=== Heavy task test ===
Number of tasks: 10, Complexity: 10000
BS::thread_pool: 735 us (0.735 ms) (Result: 142441)
My thread pool:    729 us (0.729 ms) (Result: 142441)

Test completed!
```
# 六、高级优化方向
- 任务优先级调度
  - 使用优先队列实现高优先级任务优先执行
- Work Stealing
  - 允许空闲线程从其他线程的任务队列中“stealing”任务，提升负载均衡
- 动态线程调整
  - 根据任务负载动态增减线程数量，避免资源浪费
- 异常处理与日志
  - 捕获任务执行中的异常，防止线程意外终止，并记录日志便于调试
# 七、项目实践建议
- 测试用例设计
  - 验证并发任务执行顺序
  - 压测：模拟高并发场景下的性能
  - 边界测试：空队列、线程池关闭后提交任务等异常情况
- 性能调优
  - 使用无锁队列减少锁竞争
  - 绑定线程到特定CPU，减少上下文切换
- 拓展
  - 支持GPU任务与异构计算
# 八、代码风格
- 使用LLVM基础风格 + 自定义大括号规则
- 格式化命令：`clang-format -i **/*.cpp **/*.h`