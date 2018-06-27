#ifndef CURRENTTHREAD
#define CURRENTTHREAD

#include <stdint.h>
namespace muduo
{
       namespace CurrentThread
       {
              // internal
              extern __thread int t_cachedTid;
              extern __thread char t_tidString[32];
              extern __thread int t_tidStringLength;
              extern __thread const char* t_threadName;

              void cacheTid();
              inline int tid()
              {
                     /*
                      * __builtin_expect实际上是为了满足在大多数情况不执行跳转指令，
                      * 所以__builtin_expect仅仅是告诉编译器优化，并没有改变其对真值的判断
                      */
                     if (__builtin_expect(t_cachedTid == 0, 0))
                     {
                           cacheTid();
                     }
                     return t_cachedTid;
              }

              inline const char* tidString() // for logging
              {
                     return t_tidString;
              }

              inline int tidStringLength() // for logging
              {
                     return t_tidStringLength;
              }

              inline const char* name()
              {
                     return t_threadName;
              }
              bool isMainThread();
              void sleepUsec(int64_t usec);
       }
}

#endif // CURRENTTHREAD

