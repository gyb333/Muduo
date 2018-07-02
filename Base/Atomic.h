#ifndef ATOMIC
#define ATOMIC
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include <boost/noncopyable.hpp>
#include <stdint.h>
namespace muduo
{
       namespace detail
       {
              template<typename T>
              class AtomicIntegerT : boost::noncopyable
              {
              public:
                     AtomicIntegerT()
                           : value_(0)
                     {
                     }
                     // uncomment if you need copying and assignment
                     //
                     // AtomicIntegerT(const AtomicIntegerT& that)
                     //   : value_(that.get())
                     // {}
                     //
                     // AtomicIntegerT& operator=(const AtomicIntegerT& that)
                     // {
                     //   getAndSet(that.get());
                     //   return *this;
                     // }

                     T get()    //返回value_的值，如果value_=0，把它和0交换。
                     {
                           // in gcc >= 4.7: __atomic_load_n(&value_, __ATOMIC_SEQ_CST)
                           return __sync_val_compare_and_swap(&value_, 0, 0);
                     }
                     T getAndAdd(T x)   //先获取没有修改的value_的值，再给value_+x
                     {
                           // in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
                           return __sync_fetch_and_add(&value_, x);
                     }
                     T addAndGet(T x)
                     {
                           return getAndAdd(x) + x;//因为x是局部变量，其他线程无法引用，所以+x没有问题
                     }
                     T incrementAndGet()
                     {
                           return addAndGet(1);
                     }
                     T decrementAndGet()
                     {
                           return addAndGet(-1);
                     }
                     void add(T x)
                     {
                           getAndAdd(x);
                     }
                     void increment()
                     {
                           incrementAndGet();
                     }
                     void decrement()
                     {
                           decrementAndGet();
                     }
                     T getAndSet(T newValue)    //先get然后设置为新的值
                     {
                           // in gcc >= 4.7: __atomic_exchange_n(&value, newValue, __ATOMIC_SEQ_CST)
                           return __sync_lock_test_and_set(&value_, newValue);
                     }
              private:
                     //编译器对访问该变量的代码就不再进行优化,系统总是重新从它所在的内存读取数据,而不是直接从寄存器读取
                     volatile T value_;
              };
       }
       typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
       typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}

#endif // ATOMIC

