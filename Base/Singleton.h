#ifndef SINGLETON_H
#define SINGLETON_H

#include <boost/noncopyable.hpp>
#include <assert.h>
#include <stdlib.h> // atexit
#include <pthread.h>
namespace muduo
{
namespace detail
{
// This doesn't detect inherited member functions!
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
template<typename T>
struct has_no_destroy       //不能侦测继承的成员函数
{
#ifdef __GXX_EXPERIMENTAL_CXX0X__
    template <typename C> static char test(decltype(&C::no_destroy));
#else
    template <typename C> static char test(typeof(&C::no_destroy));
#endif
    template <typename C> static int32_t test(...);

    const static bool value = sizeof(test<T>(0)) == 1;//判断如果是类的话，是否有no_destroy方法
};
}
template<typename T>
class Singleton : boost::noncopyable //线程安全的单例模式类
{
public:
    static T& instance()
    {
        //第一次调用会在init函数内部创建，pthread_once保证该函数只被调用一次！！！！
       //并且pthread_once()能保证线程安全，效率高于mutex
        pthread_once(&ponce_, &Singleton::init);
        assert(value_ != NULL);
        return *value_;//利用pthread_once只构造一次对象
    }
private:
    Singleton();
    ~Singleton();
    static void init()
    {
        value_ = new T();   //直接调用构造函数
        if (!detail::has_no_destroy<T>::value)//当参数是类且没有"no_destroy"方法才会注册atexit的destroy
        {
            ::atexit(destroy);//登记atexit时调用的销毁函数，防止内存泄漏
        }
    }
    static void destroy()//程序结束后自动调用该函数销毁
    {
        //用typedef定义了一个数组类型，数组的大小不能为-1，利用这个方法，如果是不完全类型，编译阶段就会发现错误
        typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
        //要销毁这个类型，这个类型必须是完全类型
        T_must_be_complete_type dummy; (void)dummy;
        delete value_;
        value_ = NULL;
    }
private:
    static pthread_once_t ponce_;
    static T*             value_;
};
template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;//初始化pthread_once
template<typename T>
T* Singleton<T>::value_ = NULL;//静态成员外部会初始化为空
}

#endif // SINGLETON_H
