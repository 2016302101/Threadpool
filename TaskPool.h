/** 
 * 任务池模型，TaskPool.h
 * gl 2022.03.12
 */
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>
#include <memory>
#include <iostream>

// 定义一个任务类
class Task
{
public:
    virtual void doIt()
    {
        std::cout << "handle a task..." << std::endl;
    }

    virtual ~Task()
    {
        //为了看到一个task的销毁，这里刻意补上其析构函数
        std::cout << "a task destructed..." << std::endl;
    }
};

// 定义一个线程池类
// final表示该类是最后的类，不能继承该类
class TaskPool final
{
public:
    TaskPool();
    ~TaskPool();
    TaskPool(const TaskPool& rhs) = delete;
    TaskPool& operator=(const TaskPool& rhs) = delete;

public:
    void init(int threadNum = 5); // 初始化线程，默认数量为5
    void stop(); // 清空线程池

    void addTask(Task* task); // 将任务分配给线程池
    void removeAllTasks();    // 清空所有任务

private:
    void threadFunc(); // 线程函数处理task

private:
    std::list<std::shared_ptr<Task>>            m_taskList;   // 任务队列
    std::mutex                                  m_mutexList;  // 对于任务队列设置的互斥量
    std::condition_variable                     m_cv;         // 条件变量
    bool                                        m_bRunning;   // 线程池是否应该结束
    std::vector<std::shared_ptr<std::thread>>   m_threads;    // 线程池中的线程清单
};