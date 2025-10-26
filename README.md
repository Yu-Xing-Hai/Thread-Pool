# 一、项目说明
- 本项目实现了一个基于无锁队列的线程池，并与成熟线程池BS：：threadpool进行了对比测试
# 二、测试结果
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
# 三、项目各模块分析与介绍【自问自答方式】
## 3.1  任务队列模块
### 为什么抛弃queue转而使用双向链表实现任务队列？
- queue对象作为一个整体，相较于双向链表的每个节点，操作粒度更低（如push任务时的扩缩容等），进而导致性能优化空间小
### 为什么Push和Pop操作放弃锁机制而采用CAS实现？
- 无锁队列的Push和Pop操作基于CAS（Compare-And-Swap），避免了锁机制导致的线程上下文切换开销，提高了线程并发性能（尤其在小任务高并发场景下）
### 你是如何保证操作资源(任务)的原子性的？
- 无锁队列的Push和Pop操作基于CAS（Compare-And-Swap），确保了操作资源(任务)的原子性
- 同时，我们使用atomic对象来维护队列的头指针和尾指针，确保了操作链表指针时的原子性，保证了线程安全与资源一致性
### 简单讲讲CAS操作
- CAS操作是一种原子操作，用于在多线程环境下对共享变量进行原子更新
- 它包含三个操作数：内存位置V、预期值A和新值B
- CAS操作的基本思想是：如果内存位置V的值与预期值A相等，那么将内存位置V的值更新为新值B
- CAS操作是一个原子操作，不会被其他线程中断，因此可以确保在多线程环境下对共享变量的原子更新
### 你的Pop和Push操作是怎么实现的？
- C++中，atomic库提供了compare_exchange_weak这种CAS操作
  - `old_tail->next.compare_exchange_weak(null_ptr, new_node)`
  - compare_exchange_weak是一种非阻塞的CAS操作，它在CAS操作失败时不会抛出异常，而是返回一个布尔值表示操作是否成功
- 利用CAS操作，我们可以实现原子化的任务入队出队操作
## 3.2  线程池模块
### 你是怎么存储工作线程的？
- 我们使用vector容器来存储工作线程对象（即std::thread对象）
### 你传入任务队列的任务类型是什么？
- 我们传入任务队列的任务类型是std::function<void()>，即无参数无返回值的函数对象(通过bind函数实现参数绑定)
### 你的submit函数是怎么实现的？
```C++
template <typename F, typename... Args>
auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
      );
    std::future<return_type> result = task->get_future();
    taskQueue.Push([task]() { (*task)(); });
    return result;
}
```
## 3.3  主函数模块
### 如何获取到线程执行结果？
- 我们可以通过std::future对象的get()方法来获取线程执行结果或wait()方法来等待线程执行完成