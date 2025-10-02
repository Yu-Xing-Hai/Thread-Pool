#include <atomic>
#include <utility>

// 无锁队列节点结构
template <typename T> struct LockFreeQueueNode {
    T data;
    std::atomic<LockFreeQueueNode<T>*> next; // 指向下一个节点的指针(对其的操作保持原子性)

    explicit LockFreeQueueNode(T value) : data(std::move(value)), next(nullptr) {} // 构造函数
};

// 无锁队列类实现
template <typename T> class LockFreeQueue {
  private:
    // 头指针和尾指针，使用节点的原子类型
    std::atomic<LockFreeQueueNode<T>*> head_;
    std::atomic<LockFreeQueueNode<T>*> tail_;

  public:
    LockFreeQueue() {
        // 创建一个哨兵节点作为头节点
        LockFreeQueueNode<T>* dummy_node = new LockFreeQueueNode<T>(T());  // T() 的作用是生成 T 类型的默认值
        head_.store(dummy_node);
        tail_.store(dummy_node); // 简化边界条件处理
    }

    ~LockFreeQueue() {
        // 析构函数，释放所有节点
        T temp; // 用于存储弹出的数据
        while (Pop(temp)) {
        }; // 弹出所有节点
        delete head_.load();
    }

    /*禁用拷贝
     *1.明确禁止编译器自动生成拷贝构造函数
     *2.明确禁止编译器自动生成赋值运算符，确保类的不可复制性
     *3.确保类的不可复制性
     */
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    // 入队操作
    void Push(T value) {
        // 创建新节点
        LockFreeQueueNode<T>* new_node = new LockFreeQueueNode<T>(std::move(value));
        LockFreeQueueNode<T>* old_tail = nullptr;
        LockFreeQueueNode<T>* null_ptr = nullptr;

        while (true) {
            old_tail = tail_.load();

            // 尝试将新节点链接到当前尾节点的next指针
            if (old_tail->next.compare_exchange_weak(null_ptr, new_node)) {
                break; // 成功插入，跳出循环
            } else {
                // 确保tail_指向最新的尾节点
                tail_.compare_exchange_weak(old_tail, old_tail->next.load());
            }
        }
        // 更新tail_指向新的尾节点
        tail_.compare_exchange_weak(old_tail, new_node);
    }

    // 出队操作：成功则返回true并将返回值存储到result中，失败返回false
    bool Pop(T& result) {
        // 创建新节点
        LockFreeQueueNode<T>* old_head = nullptr;

        while (true) {
            old_head = head_.load();
            LockFreeQueueNode<T>* old_tail = tail_.load();
            LockFreeQueueNode<T>* next_node = old_head->next.load();

            // 检查队列是否为空
            if (old_head == old_tail) {
                if (next_node == nullptr) {
                    return false; // 队列为空
                }

                // 尝试更新tail_指向新的尾节点
                tail_.compare_exchange_weak(old_tail, next_node);
            } else {
                // 尝试获取下一个节点的数据
                result = next_node->data;
                // 移动头指针
                if (head_.compare_exchange_weak(old_head, next_node)) {
                    break; // 成功出队，跳出循环
                }
            }
        }
        // 释放旧的头节点
        delete old_head;
        return true;
    }

    /*
    * 函数作用：检查队列是否为空
    * const说明：
    *   is_empty() const 中的 const 是对函数行为的契约式声明：
    *       对编译器：保证不修改成员变量，触发编译期检查。
    *       对开发者：明确这是只读操作，提升代码可读性。
    *       对使用者：允许在常量对象上调用，扩展函数的适用场景
    */
    bool isEmpty() const {
        LockFreeQueueNode<T>* head = head_.load();
        LockFreeQueueNode<T>* tail = tail_.load();
        // 队列为空的条件：头指针和尾指针指向同一个节点，且该节点的next指针为空
        return (head == tail) && (head->next == nullptr);
    }
}