/*
创建线程：
要创建一个线程，需要实例化 std::thread 类，并传递一个可调用对象（函数、lambda 表达式或对象的成员函数）作为参数。
*/
#include <thread>
#include <iostream>
#include <functional>

int main() {
	std::function<void()> func1 = []() {std::cout << "threadOne test!" << std::endl; };
	std::function<void()> func2 = []() {std::cout << "threadTwo test!" << std::endl; };
	std::thread threadOne(func1);  //创建线程
	auto threadTwo = std::make_unique<std::thread>(func2);  //使用智能指针管理线程对象
	threadOne.join();  //阻塞当前主线程，直到threadOne线程完成执行
	threadTwo->join();
	std::cout << "threadOne has finished!" << std::endl;
 
	return 0;
}