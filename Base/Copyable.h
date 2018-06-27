#ifndef COPYABLE
#define COPYABLE
namespace muduo
{
       /// A tag class emphasises the objects are copyable.
       /// The empty base class optimization applies.
       /// Any derived class of copyable should be a value type.
       class copyable
       {
       };
};
#endif // COPYABLE

