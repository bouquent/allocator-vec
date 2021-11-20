#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP

#include "myconstruct.hpp"

template<class T>
class allocator
{
public:
    static T* allocate();
    static T* allocate(std::size_t n);
    static void deallocate(T*);
    static void deallocate(T*, std::size_t n);

    static void construct(T*);
    static void construct(T*, const T&);
    static void construct(T*, T&&);
    template<class... Args>
    static void construct(T*, Args&&... args);
    static void destroy(T*);
    static void destroy(T*, T*);

private:
};

template<class T>
T* allocator<T>::allocate()
{
    return static_cast<T*>(::operator new(sizeof(T)));
}

template<class T>
T* allocator<T>::allocate(std::size_t n)
{
    return static_cast<T*>(::operator new(sizeof(T) * n));
}

template<class T>
void allocator<T>::deallocate(T* ptr)
{
    if (ptr == nullptr) return ;
    ::operator delete(ptr);
}

template<class T>
void allocator<T>::deallocate(T* ptr, std::size_t n)
{
    if (ptr == nullptr) return ;
    ::operator delete(ptr);
}

template<class T>
void allocator<T>::construct(T* ptr)
{
    myconstruct::construct(ptr);
}

template<class T>
void allocator<T>::construct(T* ptr, const T& value)
{
    myconstruct::construct(ptr, value);
}

template<class T>
void allocator<T>::construct(T* ptr, T&& value)  //这里的value是右值引用，但右值引用其实是左值
{
    myconstruct::construct(ptr, std::move(value));  //所以这里需要使用move将左值强制转换成右值
}

template<class T>
template<class... Args>
void allocator<T>::construct(T* ptr, Args&&... args)
{
    myconstruct::construct(ptr, std::forward<Args>(args)...);
}

/*
*   template<class T>       可以代替const T&, T&& 两个重载函数的方法
*   template<class Ty2>
*   void allocator<T>::construct(T* ptr, Ty2&& value)
*   {
*       std::cout << "allocator::construct(T*,Ty2&&)!\n";
*       myconstruct::construct(ptr, std::forward<Ty2>(value));
*   }
*
*/

template<class T>
void allocator<T>::destroy(T* ptr)
{
    myconstruct::destroy(ptr);
}

template<class T>
void allocator<T>::destroy(T* first, T* last)
{
    myconstruct::destroy(first, last);
}


#endif