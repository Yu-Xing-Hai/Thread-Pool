# 一、任务队列简述
1. 定义
- 任务队列是暂存待执行任务单元的一种缓冲区
- 生产者(提交任务的线程)向任务队列中提交任务
- 消费者(工作线程)则从任务队列中提取任务进行处理

2. 核心作用
- 解耦生产者和消费者
  - 生产者无需等待任务执行完成(队列非满状态)即可向任务队列提交任务
- 流量削峰
  - 任务队列可缓冲突发的大量任务，避免系统过载
- 资源调度
  - 通过队列管理任务的优先级、延迟执行等逻辑

3. 核心性质
- 线程安全性
  - 队列的入队、出队需保证原子性(可通过锁机制或无锁数据结构实现)
- 阻塞与非阻塞机制
  - 阻塞机制
    - 当队列为空时，消费者需自阻塞，直到队列不为空
  - 非阻塞机制
    - 直接返回空值或错误状态（适用于实时性要求高的场景）
  - 【拓展】
    - 在TCP的流量控制机制中，当发送方调用write()函数向TCP的缓冲区写数据时，同样存在阻塞与非阻塞机制
- 任务调度策略
  - 支持FIFO、优先级调度或定时任务(如延迟执行)
# 二、任务队列的实现
## 2.1  数据结构的封装
### 2.1.1  任务封装方式
- 第一种：函数对象形式——使用`std::function<void()>`封装任务【适用于C++】
  - 定义
    - `std::function<void()>`是一个通用函数包装器，用于存储和调用任何可调用对象，如函数、Lambda表达式、函数对象等，只要该对象的签名与void()匹配，它是现代C++中实现回调、事件处理等功能的核心工具
      - 使用示例请见Visual Stdio
  - 核心特性
    - 通用性：可包装任意类型的可调用对象，如普通函数、Lambda表达式、成员函数等
    - 类型擦除：隐藏了底层可调用对象的具体类型，提供统一的调用接口（即通过 operator() 调用）
    - 安全封装：若未存储任何对象时调用，会抛出异常
- 第二种：结构体/类封装形式——定义Task结构体，包含函数指针和参数【适用于C风格实现】
### 2.1.2  队列容器的选择
- 普通队列
  - std::queue：默认FIFO
  - std::deque：双端队列，支持两端操作
- 优先队列
  - std::priority_queue：需要**自定义比较函数**实现优先级调度
- 线程安全队列
  - moodycamel::ConcurrentQueue：无锁实现，减少锁竞争
## 2.2 实现步骤与代码示例
```C++
class ThreadSafeQueue {
private:
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;

public:
    void push(std::function<void()> task) {
        std::unique_lock<std::mutex> lock(mtx);
        tasks.push(std::move(task));
        cv.notify_one();  // 唤醒一个等待线程（网页3/7代码片段）
    }

    std::function<void()> pop() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !tasks.empty(); }); // 阻塞直到队列非空
        auto task = std::move(tasks.front());
        tasks.pop();
        return task;
    }
};
```
# 三、优化方向
## 3.1  动态扩容
## 3.2  **无锁队列**
## 3.3  任务优先级调度