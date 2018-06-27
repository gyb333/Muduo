#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

#include <boost/config.hpp>

namespace boost {
       //  Private copy constructor and copy assignment ensure classes derived from
       //  class noncopyable cannot be copied.
       //  Contributed by Dave Abrahams
       namespace noncopyable_  // protection from unintended ADL
       {
              class NonCopyable
              {
              protected:
#if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(BOOST_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS)
                     BOOST_CONSTEXPR NonCopyable() = default;
                     ~NonCopyable() = default;
#else
                     NonCopyable() {}
                     ~NonCopyable() {}
#endif
#if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS)
                     NonCopyable(const NonCopyable&) = delete;
                     NonCopyable& operator=(const NonCopyable&) = delete;
#else
              private:  // emphasize the following members are private
                     NonCopyable(const noncopyable&);
                     NonCopyable& operator=(const noncopyable&);
#endif
              };
       }
       typedef noncopyable_::NonCopyable noncopyable;
} // namespace boost

#endif // NONCOPYABLE_H
