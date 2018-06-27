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
    void put(const T& x)
    {
        {
            MutexLockGuard lock(mutex_);
            queue_.push_back(x);
        }
        notEmpty_.notify();
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
    T take()
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
        T front(queue_.front());    //初始化类型T
#endif

        queue_.pop_front();
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

