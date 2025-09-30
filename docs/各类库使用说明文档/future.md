# 一、< future >库的概述
- C++ 的 `<future>` 库为多线程异步编程提供了一套强大的工具，它通过标准化接口简化了线程间结果传递、异常处理和同步的复杂性
# 二、核心组件
- `<future>` 库的核心围绕三个类展开，构成异步编程的「铁三角」
  - `std::promise`
    - 生产者——设置值/异常
  - `std::future`
    - 消费者——获取结果
  - `std::packaged_task`
    - 包装任务并关联future
# 三、核心类详解
## 3.1  std::future< T >
- 定义
  - 异步结果占位符​
- 作用
  - 表示“可能在将来可用”的结果的“提货单”
- 关键方法
  - `get()`
    - 阻塞并等待结果(只能调用一次)
  - `wait`
    - 阻塞直到结果就绪
  - `wait_for(duration)`
    - 超时等待
  - `valid()`
    - 检查是否关联有效结果
- 注意点
  - **future只能移动不能复制**
  - shared_future允许多次访问
- 生命周期陷阱
  - `future` 析构时若未调用 `get()`，会隐式等待异步操作完成，可能导致意外阻塞
## 3.2  std::promise< T >
- 定义
  - 主动控制结果​
- 作用
  - 允许手动设置某个任务的最终结果(或异常)，可跨越线程传递
- 关键方法
  - `void set_value(T);`
    - 设置成功结果
  - `void set_exception(std::exception_ptr);`
    - 设置异常
  - `future<T> get_future()`
    - 获取关联的future
## 3.3  std::packaged_task< Func >
- 定义
  - 函数包装器​
- 作用
  - 将可调用对象(函数、lambda表达式等)包装为可异步执行的任务，自动关联future
- 核心优势
  - 比手动管理promise更安全，自动处理结果绑定
## 3.4  std::async
- 定义
  - 自动任务分发(asynchronous)
- 作用
  - 快捷方式创建异步任务，可选择是否启动新线程
  - 创建 `std::promise<int>` 和关联的 `std::future<int>`
  - 根据策略（默认std::launch::async|deferred）决定是否启动新线程
  - 重要特性​​：返回值类型自动推导
- 底层本质
  - 相当于封装的 `packaged_task + 线程池管理`的优化版本。
# 四、功能使用示例
## 4.1  异步结果传递
```C++
std::future<int> fut = std::async([]{
  std::this_thread::sleep_for(1s);
  return 42;
});
// do other work...
int result = fut.get();  // 阻塞获取结果
```
## 4.2  异常跨线程传播
```C++
//异步任务中的异常能通过future传递到调用线程
auto fut = std::async([] {
  throw std::runtime_error("Oops");
  return 42;
});

try {
  fut.get();  //抛出std::runtime_error
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;  //捕获并处理异常
}
```
## 4.3  多任务并行加速
```C++
auto fut1 = std::async([] {return compute_part1();});
auto fut2 = std::async([] {return compute_part2();});

int total = fut1.get() + fut2.get();  //总时间 = 最慢的任务花费时间
```
# 五、陷阱规避
## 5.1  陷阱1：丢失任务所有权
```C++
//错误示范
std::packaged_task<int()> task([] {return 42;});
std::thread t(std::ref(task));  //错误！ task必须移动而非引用
t.join();
auto result = task.get_future().get();  //未定义行为

//正确做法
std::packaged_task<int()> task([] {return 42;});
std::thread t(std::move(task));  //错误！ task必须移动而非引用
t.join();
auto result = task.get_future().get();  //未定义行为
```
# 六、性能优化
 ## 4.1  批量获取结果
 ```C++
 //错误做法：立即阻塞
 for(auto& task : tasks) task.get();  //相当于串行执行，没有利用多核性能，总耗时 = 所有任务耗时之和

 //正确做法：先启动所有任务，再统一获取结果
std::vector<std::future<int>> futures;
for(int i = 0; i < 10; i++) {  //提交10个异步任务
  futures.push_back(std::async(...));  //`...`表示函数对象或lambda表达式
}
for(auto& f : futures) f.get();
 ```
# 七、常见疑问与解答