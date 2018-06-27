#ifndef TYPES_H
#define TYPES_H

/*stdint.h中定义了标准的扩展整数类型，包括准确长度类型，
 * 最小长度类型、快速长度类型、最大长度类型
 * 增强程序可读性， uint8_t; ,8位无符号。uint16_t  16位无符号。
 * unsigned   char   这个，在不同的情况下，长度是不确定的。
 * */
#include <stdint.h>

#ifdef MUDUO_STD_STRING
#include <string>
#else
#include <ext/vstring.h>
#include <ext/vstring_fwd.h>
#endif

#ifndef NDEBUG
#include <assert.h>
#endif

namespace muduo {
    #ifdef MUDUO_STD_STRING
    using std::string;
    #else
    typedef __gnu_cxx::__sso_string string;
    #endif

template<typename To,typename From>
inline To implicit_cast(From const &f){
    return f;
}

template<typename To,typename From>
inline To down_cast(From* f){
    if(false){
        implicit_cast<From*,To>(0);
    }
    #if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
        assert(f==NULL||dynamic_cast<To>(f)!=NULL);
    #endif
        return static_cast<To>(f);
}

}




#endif // TYPES_H
