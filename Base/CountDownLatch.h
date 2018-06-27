#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H

#include <Base/Condition.h>
#include <Base/Mutex.h>
#include <boost/noncopyable.hpp>
namespace muduo
{
       class CountDownLatch : boost::noncopyable
       {
       public:
              explicit CountDownLatch(int count);
              void wait();
              void countDown();
              int getCount() const;
       private:
              mutable MutexLock mutex_;
              Condition condition_;
              int count_;
       };
}


#endif // COUNTDOWNLATCH_H
