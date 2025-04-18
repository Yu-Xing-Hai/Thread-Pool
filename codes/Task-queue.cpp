/*
目标：实现任务队列【第一版】
功能：用于暂存任务(待执行函数)
核心实现：线程安全性、阻塞与非阻塞
*/
#include <queue>
#include <functional>  //通用函数包装器的库
#include <mutex>       //互斥锁的库
#include <condition_variable>  //线程同步工具，用于阻塞或唤醒线程
#include <utility>  //包含movd()函数的库
#include <iostream>

class TasksQueue {
private:
	std::queue<std::function<void()>> tasks;  //第一阶段：使用FIFO队列实现
	std::mutex mtx;  //访问任务队列的互斥锁【当前消费者和生产者共用一把锁】
	std::condition_variable cv;  //
public:
	void Push(std::function<void()> task) {
		std::unique_lock<std::mutex> lock(mtx);  //生产者获取进入任务队列的锁
		/*未考虑任务队列空间有限情况*/
		tasks.push(std::move(task));  //任务入队,move函数用于转移对象持有者【待补充】
		cv.notify_one();  //唤醒一个消费者线程
	};

	std::function<void()> Pop() {
		std::unique_lock<std::mutex> lock(mtx);  //消费者获取进入任务队列的锁
		cv.wait(lock, [this](){return !tasks.empty();});  //如果任务队列为空，则阻塞当前进程并自动释放锁
		std::function<void()> task = std::move(tasks.front());
		tasks.pop();
		return task;
		/*std::unique_lock<std::mutex>遵从RAII规则，离开作用域后自动调用析构函数释放mtx*/
	};
};

int main() {
	/*实例化任务队列：使用智能指针自动管理内存*/
	std::unique_ptr<TasksQueue> tasksQueue = std::make_unique<TasksQueue>();

	/*实现生产者*/
	//1. 生产任务一
	std::function<void()> task1 = ([]() {std::cout << "Hello! I am first task!" << std::endl; });

	tasksQueue->Push(task1);

	/*实现消费者*/
	std::function<void()> consumer = tasksQueue->Pop();
	consumer();

	return 0;
};