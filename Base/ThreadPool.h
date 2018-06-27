#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <Base/Condition.h>
#include <Base/Mutex.h>
#include <Base/Thread.h>
#include <Base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include <deque>

namespace muduo
{

class ThreadPool : boost::noncopyable
{
public:
    // 线程池的任务，线程池的每一个线程都可以执行一个或多个任务
    typedef boost::function<void ()> Task;

    explicit ThreadPool(const string& nameArg = string("ThreadPool"));
    ~ThreadPool();

    // Must be called before start().
    // 设置任务队列的最大长度
    void setMaxQueueSize(int maxSize) {
        maxQueueSize_ = maxSize;
    }

    // 设置线程池的初始化回调函数
    void setThreadInitCallback(const Task& cb)
    {
        threadInitCallback_ = cb;
    }

    // 启动线程池，并创建指定数量的线程
    void start(int numThreads);

    // 停止线程池的运行
    void stop();

    // 线程池的名字
    const string& name() const
    {
        return name_;
    }

    // 任务队列的任务数量
    size_t queueSize() const;

    // Could block if maxQueueSize > 0
    // 运行一个任务（只是把它放进任务队列）
    void run(const Task& f);
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    void run(Task&& f);
#endif

private:
    bool isFull() const;    // 任务队列是否已满

    // 线程的运行函数：在这个函数里，线程取出任务，然后执行任务
    void runInThread();

    // 从任务队列中取出一个任务
    Task take();

    mutable MutexLock mutex_;    // 锁，用于下面的两个条件变量
    Condition notEmpty_;        // 通知其他人，任务队列非空，可以从中取出任务
    Condition notFull_;         // 通知其他人，任务队列没有满，可以添加任务
    string name_;               // 线程池名字
    Task threadInitCallback_;    // 用于初始化的任务

    //所用于管理动态分配对象的容器，容器在析构的时候，会自动清理指针
    boost::ptr_vector<muduo::Thread> threads_;   // 线程列表
    std::deque<Task> queue_;    // 任务队列
    size_t maxQueueSize_;       // 队列的最大任务数
    bool running_;          // 线程池是否正在运行
};

}

#endif // THREADPOOL_H
