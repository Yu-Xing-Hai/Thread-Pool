# 一、exception标准异常基类定义
- `std::exception`是所有标准异常类的基类，子类有以下类型
  - `std::runtime_error`：运行时错误
  - `std::logic_error`：逻辑错误
  - `std::bad_alloc`：内存分配失败
  - `std::out_of_range`：越界访问
- 利用try-catch块可捕获和处理异常
- 通过捕获exception，可以国立大多数标准库和用户自定义的异常
# 二、使用方法
## 2.1  基本语法
```C++
try {
    //可能抛出异常的代码
} catch (const std::exception& e) {  //捕获所有派生自std::exception的异常0
    std::cout << "Catch exception：" << e.what() << std::endl;  //`.what()`函数用于返回异常的描述信息
}
```
## 2.2  捕获自定义异常
```C++
#include <iostream>
#include <stdexcept>

class MyException : public std::exception {  //自定义异常类继承自exception基类
public:
    /*
    const char*：返回 C 风格字符串（异常描述信息）避免动态内存分配因内存不足导致分配异常
        char* 是指向字符的指针​​，但 C 风格字符串以 \0（空字符）结尾，其长度由 \0 的位置决定。
        const保证返回的是静态字符串，即字符串字面量存储在程序的只读数据段中，无需担心字符串存储越界问题
    what()：函数名，覆盖基类的虚函数
    const：成员函数不会修改对象状态（const 成员函数）。
    noexcept：承诺该函数不会抛出异常（C++11 起要求必须写）
    override：显式标记覆盖基类虚函数（避免拼写错误）。
    */
    const char* what() const noexcept override {
        return "My custom exception!";
    }
};

int main() {
    try {
        throw MyException();  // 抛出自定义异常
    } catch (const std::exception& e) {
        std::cerr << "Caught: " << e.what() << std::endl;
    }

    return 0;
}
```
# 三、注意点
- 优先捕获具体异常
- 避免空catch块
- 不要滥用`catch(...)`
  - 此为“捕获所有”的语法，应尽量避免，因为它无法获取异常信息
- 自定义异常应继承，以便统一处理
# 四、常见问题
## 4.1  为什么 catch (const std::exception& e) 优于 catch (std::exception e)？
- 引用 (&)​​ 
  - 避免拷贝，提高效率
- const​​
  - 防止修改异常对象