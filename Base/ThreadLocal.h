#ifndef THREADLOCAL_H
#define THREADLOCAL_H

#include <Base/Mutex.h>  // MCHECK
#include <boost/noncopyable.hpp>
#include <pthread.h>
namespace muduo
{
template<typename T>
class ThreadLocal : boost::noncopyable
{
public:
    ThreadLocal()
    {
        //构造函数中创建key，数据的销毁由destructor来销毁
        MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
    }

    ~ThreadLocal()
    {
        MCHECK(pthread_key_delete(pkey_));  //析构函数中销毁key
    }

    T& value()      //获取线程特定数据
    {
        T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));//通过key获取线程特定数据
        if (!perThreadValue)//如果是空的，说明特定数据还没有创建，那么就空构造一个
        {
            T* newObj = new T();
            MCHECK(pthread_setspecific(pkey_, newObj));//设置特定数据
            perThreadValue = newObj;
        }
        return *perThreadValue;//返回对象引用，所以需要*
    }
private:
    static void destructor(void *x)
    {
        T* obj = static_cast<T*>(x);
        //检测是否是完全类型
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        T_must_be_complete_type dummy; (void)dummy;
        delete obj;
    }
private:
    pthread_key_t pkey_;
};
}

#endif // THREADLOCAL_H
