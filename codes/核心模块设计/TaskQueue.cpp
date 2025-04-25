/*
[第二版]C++线程池实现
当前目标：
1. 实现有界任务队列 【√】
2. 实现异常处理

后续规划
1. 除非性能测试证明锁竞争是瓶颈时，采用双互斥锁
*/
#include <queue>
#include <functional>  //通用函数包装器的库
#include <mutex>       //互斥锁的库
#include <condition_variable>  //线程同步工具，用于阻塞或唤醒线程
#include <iostream>

#define MAX_QUEUE_SIZE 100

class TasksQueue {
private:
	std::queue<std::function<void()>> tasks;  //第一阶段：使用FIFO队列实现
	std::mutex mtx;  //访问任务队列的互斥锁【当前消费者和生产者共用一把锁】
	std::condition_variable cv_producer;  //生产者条件变量
	std::condition_variable cv_consumer;  //消费者条件变量
public:
	void Push(std::function<void()> task) {
		std::unique_lock<std::mutex> lock(mtx);  //生产者获取进入任务队列的锁
		cv_producer.wait(lock, [this]() {return tasks.size() <= MAX_QUEUE_SIZE; });  //限制任务队列容量，wait函数第二个参数为true时才可向下执行，否则释放锁并阻塞当前线程
		tasks.push(std::move(task));  //任务入队,move函数用于转移函数对象持有者，避免函数对象的拷贝开销				
		cv_consumer.notify_one();  //唤醒一个消费者线程
	};

	std::function<void()> Pop() {
		std::unique_lock<std::mutex> lock(mtx);  //消费者获取进入任务队列的锁
		cv_consumer.wait(lock, [this](){return !tasks.empty();});  //如果任务队列为空，则阻塞当前进程并自动释放锁
		std::function<void()> task = std::move(tasks.front());
		tasks.pop();
		cv_producer.notify_one();  //唤醒一个生产者
		return task;
		/*std::unique_lock<std::mutex>遵从RAII规则，离开作用域后自动调用析构函数释放lock*/
	};
};