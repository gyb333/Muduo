#ifndef THREAD_H
#define THREAD_H



#include <Base/Atomic.h>
#include <Base/CountDownLatch.h>
#include <Base/Types.h>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <pthread.h>

namespace muduo
{
       class Thread : boost::noncopyable
       {
       public:
              typedef boost::function<void()> ThreadFunc;
              explicit Thread(const ThreadFunc&, const string& name = string());
#ifdef __GXX_EXPERIMENTAL_CXX0X__
           //右值引用，在对象返回的时候不会拷贝构造临时对象，而是和临时对象交换，提高了效率
              explicit Thread(ThreadFunc&&, const string& name = string());
#endif
              ~Thread();
              void start();
              int join(); // return pthread_join()
              bool started() const { return started_; }
              // pthread_t pthreadId() const { return pthreadId_; }
              pid_t tid() const { return tid_; }
              const string& name() const { return name_; }
              static int numCreated() { return numCreated_.get(); }
       private:
              void setDefaultName();
              bool       started_;
              bool       joined_;
              pthread_t  pthreadId_;
              pid_t      tid_;
              ThreadFunc func_; //线程回调函数
              string     name_;
              CountDownLatch latch_;
              static AtomicInt32 numCreated_;   //原子操作
       };
}

#endif // THREAD_H
