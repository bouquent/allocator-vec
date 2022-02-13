#ifndef MYALLOC_H
#define MYALLOC_H

#include "alloc.h"
#include "myconstruct.h"

template<typename T>
class myalloc
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef size_t type_size;
    typedef ptrdiff_t difference_type;

    static T* allocate();
    static T* allocate(size_t n);
    static void deallocate(T* ptr);
    static void deallocate(T* ptr, size_t n);

    template<typename... Args>
    static void construct(T *ptr, Args... args);

    static void destroy(T* ptr);
    static void destroy(T* ptr1, T* ptr2);
private:
    static alloc alloc_;
};

template<typename T>
alloc myalloc<T>::alloc_;

template<typename T>
T* myalloc<T>::allocate()
{
    return (T*)alloc_.allocate(sizeof(T));
}

template<typename T>
T* myalloc<T>::allocate(size_t n)
{
    if (n == 0) return nullptr;
    return (T*)alloc_.allocate(sizeof(T) * n);
}

template<typename T>
void myalloc<T>::deallocate(T* ptr)
{
    alloc_.deallocate(ptr, sizeof(T));
}

template<typename T>
void myalloc<T>::deallocate(T* ptr, size_t n)
{
    alloc_.deallocate(ptr, sizeof(T) * n);
} 

template<typename T>
template<typename... Args>
void myalloc<T>::construct(T* ptr, Args... args)
{
    myconstruct::construct(ptr, std::forward<Args>(args)...);
}

template<typename T>
void myalloc<T>::destroy(T* ptr) 
{
    myconstruct::destory(ptr);
}

template<typename T>
void myalloc<T>::destroy(T* ptr1, T* ptr2) 
{
    myconstruct::destory(ptr1, ptr2);
}

#endif 