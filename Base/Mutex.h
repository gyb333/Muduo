#ifndef MUTEX
#define MUTEX

#include <Base/CurrentThread.h>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include <pthread.h>
#ifdef CHECK_PTHREAD_RETURN_VALUE
#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail(int errnum,
       const char *file,
       unsigned int line,
       const char *function)
       __THROW __attribute__((__noreturn__));
__END_DECLS
#endif
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})
#else  // CHECK_PTHREAD_RETURN_VALUE
//MCHECK()宏的实现,进行一致性检查。它可以检查出内存分配不匹配的情况。返回值即可能发生的错误
#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})
#endif // CHECK_PTHREAD_RETURN_VALUE
namespace muduo
{
       class MutexLock : boost::noncopyable
       {
       public:
              MutexLock()
                     : holder_(0)
              {
                     MCHECK(pthread_mutex_init(&mutex_, NULL));//MEMCHECK是多retval的检测，相当于assert
              }
              ~MutexLock()
              {
                     assert(holder_ == 0); //只有在没有被其它线程持有的情况下才可以析构
                     MCHECK(pthread_mutex_destroy(&mutex_));
              }
              // must be called when locked, i.e. for assertion
              bool isLockedByThisThread() const //是否被本线程上锁
              {
                     return holder_ == CurrentThread::tid();//返回id，通过systemcall + cache方式
              }

              void assertLocked() const
              {
                     assert(isLockedByThisThread());
              }

              // internal usage
              void lock()
              {
                     MCHECK(pthread_mutex_lock(&mutex_));
                     assignHolder();
              }
              void unlock()
              {
                     unassignHolder();
                     MCHECK(pthread_mutex_unlock(&mutex_));
              }
              pthread_mutex_t* getPthreadMutex() /* non-const */
              {
                     return &mutex_;
              }
       private:

              friend class Condition;
              class UnassignGuard : boost::noncopyable  //取消赋值
              {
              public:
                     UnassignGuard(MutexLock& owner)
                           : owner_(owner)
                     {
                           owner_.unassignHolder();
                     }
                     ~UnassignGuard()
                     {
                           owner_.assignHolder();
                     }
              private:
                     MutexLock& owner_;
              };



              void unassignHolder()
              {
                     holder_ = 0;
              }
              void assignHolder()
              {
                     holder_ = CurrentThread::tid();
              }
              pthread_mutex_t mutex_;
              pid_t holder_;
       };




       // Use as a stack variable, eg.
       // int Foo::size() const
       // {
       //   MutexLockGuard lock(mutex_);
       //   return data_.size();
       // }

       //利用C++的RAII机制，让锁在作用域内全自动化
       class MutexLockGuard : boost::noncopyable
       {
       public:
              explicit MutexLockGuard(MutexLock& mutex)
                     : mutex_(mutex)
              {
                     mutex_.lock();
              }
              ~MutexLockGuard()
              {
                     mutex_.unlock();
              }
       private:
              MutexLock& mutex_;//他们仅仅是关联关系，使用引用不会导致MutexLock对象的销毁
       };
}
// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

#endif // MUTEX

