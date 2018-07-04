#ifndef BLOCKINGQUEUE
#define BLOCKINGQUEUE

#include <Base/Condition.h>
#include <Base/Mutex.h>
#include <boost/noncopyable.hpp>
#include <deque>
#include <assert.h>
namespace muduo
{
template<typename T>
class BlockingQueue : boost::noncopyable
{
public:
    BlockingQueue()
        : mutex_(),
          notEmpty_(mutex_),
          queue_()
    {
    }
    void put(const T& x)    //往阻塞队列放任务
    {
        {
            MutexLockGuard lock(mutex_);
            queue_.push_back(x);
        }
        notEmpty_.notify(); //不空唤醒
    }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    void put(T&& x)
    {
        {
            MutexLockGuard lock(mutex_);
            queue_.push_back(std::move(x));
        }
        notEmpty_.notify();
    }
    // FIXME: emplace()
#endif
    T take()//往阻塞队列取任务
    {
        MutexLockGuard lock(mutex_);
        // always use a while-loop, due to spurious wakeup
        while (queue_.empty())
        {
            notEmpty_.wait();
        }
        assert(!queue_.empty());

#ifdef __GXX_EXPERIMENTAL_CXX0X__
        T front(std::move(queue_.front()));
#else
        //T x = T();   // 如果T是内建类型，x是0或者false
        //T x = queue_.front();
        T front(queue_.front());    //取出队头 初始化类型T
#endif
        //用的时候用front取出来尽管用，等到用完了在pop，就不会非法访问内存
        queue_.pop_front();//弹出队头。出队的东西是要被free掉的，就不会内存泄漏
        return front;
    }
    size_t size() const
    {
        MutexLockGuard lock(mutex_);
        return queue_.size();
    }
private:
    mutable MutexLock mutex_;     //mutable突破const限制
    Condition         notEmpty_;
    std::deque<T>     queue_;
};
}

#endif // BLOCKINGQUEUE

