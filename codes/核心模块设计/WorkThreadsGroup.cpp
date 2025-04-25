/*
【第一版】工作线程组实现
当前目标：
1. 实现一个生产者线程和两个消费者线程


后续规划：
*/
#include "TaskQueue.h"
#include <functional>
#include <thread>
#include <queue>
#include <mutex>       //互斥锁的库
#include <condition_variable>  //线程同步工具，用于阻塞或唤醒线程
#include <iostream>


class ThreadPool {
private:
	TasksQueue TaskQueue;  //实例化任务队列

	void Consumer() {  //创建两个可复用的消费者线程
		while (true) {
			//可复用消费者线程1
			std::thread consumerThread1([this]() {
				auto Task = TaskQueue.Pop();
				Task();
				});
			consumerThread1.join();

			//可复用消费者线程2
			std::thread consumerThread2([this]() {
				auto Task = TaskQueue.Pop();
				Task();
				});
			consumerThread2.join();
		}
	}
public:
	ThreadPool() {
		Consumer();  //构造时就调用消费者函数，从而执行消费者线程
	}
	void Producer(std::function<void()> Task) {
		std::thread produceThread([this, Task]() {TaskQueue.Push(Task); });  //创建生产者线程
		produceThread.join();
	}
};