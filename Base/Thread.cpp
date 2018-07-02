#include <Base/Thread.h>
#include <Base/CurrentThread.h>
#include <Base/Exception.h>
#include <Base/Logging.h>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/weak_ptr.hpp>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

/*
 * __thread是GCC内置的线程局部存储设施，存取效率可以和全局变量相比。
 * __thread变量每一个线程有一份独立实体，各个线程的值互不干扰。
 * 例如t_cachedTid这个变量，每个线程都有，但是每个线程的t_cachedTid是独立的，也就是一个线程只能修改它自己线程的t_cachedTid，而不能修改其他线程的t_cachedTid
 * 可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。
 */

namespace muduo
{
namespace CurrentThread
{
//用来缓存的id  //__thread修饰的变量是线程局部存储的，线程不共享，线程安全
__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "unknown";

const bool sameType = boost::is_same<int, pid_t>::value;  //类型是否相同
BOOST_STATIC_ASSERT(sameType);    //编译时断言错误
}

namespace detail
{
pid_t gettid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}
void afterFork()//fork之后打扫战场，子进程中执行
{
    muduo::CurrentThread::t_cachedTid = 0;
    muduo::CurrentThread::t_threadName = "main";
    //因为fork可能在主线程中调用，也可能在子线程中调用。fork得到一个新进程，新进程只有一个执行序列，只有一个线程
    CurrentThread::tid();
    //实际上服务器要么多进程，要么多线程。如果都用，甚至可能死锁
    // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer   //线程名初始化
{
public:
    ThreadNameInitializer()
    {
        muduo::CurrentThread::t_threadName = "main";
        CurrentThread::tid();
        //如果我们调用了fork函数，调用成功后子进程会调用afterfork()
        pthread_atfork(NULL, NULL, &afterFork);
    }
};
ThreadNameInitializer init;


struct ThreadData//线程数据类，观察者模式
{
    typedef muduo::Thread::ThreadFunc ThreadFunc;
    ThreadFunc func_;
    string name_;
    pid_t* tid_;
    CountDownLatch* latch_;
    ThreadData(const ThreadFunc& func,
               const string& name,
               pid_t* tid,
               CountDownLatch* latch)
        : func_(func),
          name_(name),
          tid_(tid),
          latch_(latch)
    { }

    void runInThread()
    {
        *tid_ = muduo::CurrentThread::tid();
        tid_ = NULL;
        latch_->countDown();
        latch_ = NULL;
        muduo::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();
        ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);
        try
        {
            func_();
            muduo::CurrentThread::t_threadName = "finished";
        }
        catch (const Exception& ex)
        {
            muduo::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
            abort();
        }
        catch (const std::exception& ex)
        {
            muduo::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
            fprintf(stderr, "reason: %s\n", ex.what());
            abort();
        }
        catch (...)
        {
            muduo::CurrentThread::t_threadName = "crashed";
            fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
            throw; // rethrow
        }
    }
};
void* startThread(void* obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);//派生类指针转化成基类指针，obj是派生类的this指针
    data->runInThread();
    delete data;
    return NULL;
}
}
}

using namespace muduo;
void CurrentThread::cacheTid()
{
    if (t_cachedTid == 0)
    {
        t_cachedTid = detail::gettid();
        t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}

bool CurrentThread::isMainThread()
{
    return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
    struct timespec ts = { 0, 0 };
    ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
    ::nanosleep(&ts, NULL);
}




AtomicInt32 Thread::numCreated_;
Thread::Thread(const ThreadFunc& func, const string& n)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(func),
      name_(n),
      latch_(1)
{
    setDefaultName();
}
#ifdef __GXX_EXPERIMENTAL_CXX0X__//C++11标准
Thread::Thread(ThreadFunc&& func, const string& n)
    : started_(false),
      joined_(false),
      pthreadId_(0),
      tid_(0),
      func_(std::move(func)),
      name_(n),
      latch_(1)
{
    setDefaultName();
}
#endif

//pthread_join()和pthread_detach()都是防止现成资源泄露的途径，join()会阻塞等待。
Thread::~Thread()
{
    //线程安全，析构时确认thread没有join，才会执行析构。即线程的析构不会等待线程结束
    //如果thread对象的生命周期长于线程，那么可以通过join等待线程结束。否则thread对象析构时会自动detach线程，防止资源泄露
    if (started_ && !joined_)//如果没有join，就detach，如果用过了，就不用了
    {
        pthread_detach(pthreadId_);
    }
}
void Thread::setDefaultName()   //相当于给没有名字的线程起个名字
{
    int num = numCreated_.incrementAndGet();
    if (name_.empty())
    {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start()        //线程启动
{
    assert(!started_);
    started_ = true;
    // FIXME: move(func_)
    detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
    if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
    {
        started_ = false;
        delete data; // or no delete?
        LOG_SYSFATAL << "Failed in pthread_create";
    }
    else
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}

int Thread::join()      //等待线程
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthreadId_, NULL);
}
