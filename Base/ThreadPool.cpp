#include <Base/ThreadPool.h>

#include <Base/Exception.h>

#include <boost/bind.hpp>
#include <assert.h>
#include <stdio.h>
#include <iostream>

using namespace muduo;

ThreadPool::ThreadPool(const string& nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false)
{
}

ThreadPool::~ThreadPool()
{
    if (running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);   //直接开辟所需空间，给容器预留空间
    // 创建指定数量的线程，并启动线程运行
    for (int i = 0; i < numThreads; ++i)
    {
        char id[32];//strlen(id)
        //能够控制要写入的字符串的长度，更加安全，只要稍加留意，不会造成缓冲区的溢出
        snprintf(id, sizeof id, "%d", i+1);
        threads_.push_back(new muduo::Thread(
                               boost::bind(&ThreadPool::runInThread, this), name_+id));
        threads_[i].start();
    }
     // 如果指定的线程数量为0而且初始化回调函数不为空，那么就调用初始化回调函数
    if (numThreads == 0 && threadInitCallback_)
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(mutex_);
         // 通知所有线程，停止运行，并让处于等待（等待的原因是队列中没有任务，通过notEmpty_的激活，
        //可以让线程认为队列中还有任务，都恢复运行）的线程恢复运行，并跳出循环
        running_ = false;
        notEmpty_.notifyAll();
    }

    // 等待每一个线程执行完毕
    for_each(threads_.begin(),
             threads_.end(),
             boost::bind(&muduo::Thread::join, _1));
}

// 获取任务队列的大小
size_t ThreadPool::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

// 运行任务
void ThreadPool::run(const Task& task)
{
     // 如果没有创建线程，那么将直接执行任务，阻塞方式
    if (threads_.empty())
    {
        task();
    }
    else
    {
        // 把任务放入任务队列中
        MutexLockGuard lock(mutex_);
         // 如果队列已经满了，那么这个过程将会阻塞直到队列不再满为止
        while (isFull())
        {
            notFull_.wait();
        }
        assert(!isFull());

        queue_.push_back(task);
        // 通知所有线程，队列中有任务
        notEmpty_.notify();
    }
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void ThreadPool::run(Task&& task)
{
    if (threads_.empty())
    {
        task();
    }
    else
    {
        MutexLockGuard lock(mutex_);
        while (isFull())
        {
            notFull_.wait();
        }
        assert(!isFull());

        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}
#endif

// 从任务队列中取出一个任务
ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(mutex_);
    // always use a while-loop, due to spurious wakeup
     // 如果任务队列为空，那么将会一直等到队列不为空
    while (queue_.empty() && running_)
    {
        notEmpty_.wait();   //释放占用锁资源，等待
    }
    Task task;
    if (!queue_.empty())    //任务队列不为空
    {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0)
        {
            notFull_.notify(); // 通知其他线程，任务队列不满
        }
    }
    return task;
}
// 判断任务队列是否满了
bool ThreadPool::isFull() const
{
    mutex_.assertLocked();
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

// 这个函数由线程执行，主要功能是启动一个循环，在循环里取出一个任务，然后执行任务
void ThreadPool::runInThread()
{
    try
    {
        if (threadInitCallback_)
        {
            threadInitCallback_();
        }
        while (running_)
        {
            Task task(take());
            if (task)
            {
                task();
            }
        }
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}
