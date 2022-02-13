#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "myconstruct.h"


template<typename T>
class allocator
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef size_t size_type;
    typedef ptrdiff_t  difference_type;

public:
    static T* allocate();
    static T* allocate(size_t n);

    static void deallocate(T*);
    static void deallocate(T*, size_t n);


    static void construct(T*, T);
    static void construct(T*, const T&);
    static void construct(T*, T&&);
    template<typename... Args> 
    static void construct(T*, Args&&... args);

    static void destory(T*);
    static void destory(T*, T*);
private:
};

template<typename T>
T* allocator<T>::allocate()
{
    return static_cast<T*>(::operator new(sizeof(T))); 
}

template<typename T>
T* allocator<T>::allocate(size_t n)
{
    return static_cast<T*>(::operator new(sizeof(T) * n));
} 

template<typename T>
void allocator<T>::deallocate(T* t) 
{
    if (t == nullptr) return ;
    return ::delete(t);
}

template<typename T>
void allocator<T>::deallocate(T* t, size_t n) 
{
    if (t == nullptr) return ;
    return ::delete(t);
}

template<typename T>
void allocator<T>::construct(T* ptr, T value) 
{
    myconstruct::construct(ptr, value);
} 

template<typename T>
void allocator<T>::construct(T* ptr, const T& value) 
{
    myconstruct::construct(ptr, value);
}

template<typename T>
void allocator<T>::construct(T* ptr, T&& value) 
{
    myconstruct::construct(ptr, std::forward<T>(value));
}
template<typename T>
template<typename... Args>
void allocator<T>::construct(T* ptr, Args&&... args) 
{
    myconstruct::construct(ptr, std::forward<Args>(args)...);
}

template<typename T>
void allocator<T>::destory(T* ptr)
{
    myconstruct::destory(ptr);
}

template<typename T>
void allocator<T>::destory(T* ptr1, T* ptr2) 
{
    myconstruct::destory(ptr1, ptr2);
}

#endif 