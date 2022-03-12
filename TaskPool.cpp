/**
 * �����ģ�ͣ�TaskPool.cpp
 * gl 2022.03.12
 */
#include "TaskPool.h"

// Ĭ�ϳ�ʼʱ���̳߳ز�����
TaskPool::TaskPool() : m_bRunning(false)
{

}

// �ͷ��̳߳���Դ
TaskPool::~TaskPool()
{
    removeAllTasks();
}

// ��ʼ���̣߳�Ĭ������Ϊ5
void TaskPool::init(int threadNum/* = 5*/)
{
    if (threadNum <= 0)
        threadNum = 5;

    m_bRunning = true;

    for (int i = 0; i < threadNum; ++i)
    {
        std::shared_ptr<std::thread> spThread;
        spThread.reset(new std::thread(std::bind(&TaskPool::threadFunc, this)));
        // reset�������Ϊ�գ����ǰ����use_count-1�������Ϊ�վ��½�һ��
        // ���߳�ָ�����󶨳�ʼ����ִ�е��߳�
        m_threads.push_back(spThread);
    }
}

// ��������߳� ��������Ϊ������״̬
void TaskPool::threadFunc()
{
    std::shared_ptr<Task> spTask;
    while (true)
    {
        std::unique_lock<std::mutex> guard(m_mutexList);
        // ΪʲôҪ�����whileѭ�� ��ٻ��ѵ����
        // ����̵߳ȴ�ͬһ���������� �������˼��� ����
        // ������ִ�еľ�ֻ��һ�� ����Ҫ����while ȷ��
        // �̼߳ȱ������� �������������
        while (m_taskList.empty())
        {                 
            if (!m_bRunning)
                break;
            
            //�������˻��������������������ʵĻ���pthread_cond_wait���ͷ�����������ִ�С�
            //�������仯���������ʣ�pthread_cond_wait��ֱ�ӻ������
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

// ֹͣ�̳߳صĹ��� �������߳̾��˳�
void TaskPool::stop()
{
    m_bRunning = false;
    m_cv.notify_all();

    //�ȴ������߳��˳�
    for (auto& iter : m_threads)
    {
        if (iter->joinable())
            iter->join();
    }
}

// ���̳߳���������� Ҫ��������������
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

// �ͷ��������� ��Ϊ����spָ����Щ���� ����Ҫ����Щspһ��һ��reset
// Ȼ������������
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