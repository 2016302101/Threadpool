/**
 * 任务池模型，TaskPool.cpp
 * gl 2022.03.12
 */
#include "TaskPool.h"

// 默认初始时刻线程池不工作
TaskPool::TaskPool() : m_bRunning(false)
{

}

// 释放线程池资源
TaskPool::~TaskPool()
{
    removeAllTasks();
}

// 初始化线程，默认数量为5
void TaskPool::init(int threadNum/* = 5*/)
{
    if (threadNum <= 0)
        threadNum = 5;

    m_bRunning = true;

    for (int i = 0; i < threadNum; ++i)
    {
        std::shared_ptr<std::thread> spThread;
        spThread.reset(new std::thread(std::bind(&TaskPool::threadFunc, this)));
        // reset如果参数为空，解绑当前对象，use_count-1；如果不为空就新建一个
        // 用线程指针来绑定初始化并执行的线程
        m_threads.push_back(spThread);
    }
}

// 构建多个线程 均被设置为带唤醒状态
void TaskPool::threadFunc()
{
    std::shared_ptr<Task> spTask;
    while (true)
    {
        std::unique_lock<std::mutex> guard(m_mutexList);
        // 为什么要用这个while循环 虚假唤醒的情况
        // 多个线程等待同一个条件变量 被唤醒了几个 但是
        // 最终能执行的就只有一个 所以要加入while 确保
        // 线程既被唤醒了 还有任务可以做
        while (m_taskList.empty())
        {                 
            if (!m_bRunning)
                break;
            
            //如果获得了互斥锁，但是条件不合适的话，pthread_cond_wait会释放锁，不往下执行。
            //当发生变化后，条件合适，pthread_cond_wait将直接获得锁。
            m_cv.wait(guard);
        }

        if (!m_bRunning)
            break;

        spTask = m_taskList.front();
        m_taskList.pop_front();

        if (spTask == NULL)
            continue;

        spTask->doIt();
        spTask.reset();
    }

    std::cout << "exit thread, threadID: " << std::this_thread::get_id() << std::endl;
}

// 停止线程池的工作 将所有线程均退出
void TaskPool::stop()
{
    m_bRunning = false;
    m_cv.notify_all();

    //等待所有线程退出
    for (auto& iter : m_threads)
    {
        if (iter->joinable())
            iter->join();
    }
}

// 向线程池中添加任务 要互斥访问任务队列
void TaskPool::addTask(Task* task)
{
    std::shared_ptr<Task> spTask;
    spTask.reset(task);

    {
        std::lock_guard<std::mutex> guard(m_mutexList);       
        //m_taskList.push_back(std::make_shared<Task>(task));
        m_taskList.push_back(spTask);
        std::cout << "add a Task." << std::endl;
    }
    
    m_cv.notify_one();
}

// 释放所有任务 因为是用sp指向这些任务 所以要把这些sp一个一个reset
// 然后清空这个队列
void TaskPool::removeAllTasks()
{
    {
        std::lock_guard<std::mutex> guard(m_mutexList);
        for (auto& iter : m_taskList)
        {
            iter.reset();
        }
        m_taskList.clear();
    }
}