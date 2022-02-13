#ifndef MYCONSTRUCT_H
#define MYCONSTRUCT_H

#include <new>

namespace myconstruct
{
    template<typename T>
    void construct(T* ptr) {
         ::new(ptr) T();
    }
    template<typename T> 
    void construct(T* ptr, const T& value) {
         ::new(ptr) T(value);
    }
    template<typename T>
    void construct(T* ptr, T&& value) {   //利用完美转发，可以匹配左右值
        ::new(ptr) T(std::forward<T>(value));   
    }
    template<typename T, typename... Args>
    void construct(T* ptr, Args&&... args) {
        ::new(ptr) T(std::forward<Args>(args)...);
    }
    
    template<typename T>
    void destory(T* ptr) {
        ptr->~T();
    }
    template<typename T>
    void destory(T* ptr1, T* ptr2) {
        while (ptr1 != ptr2) {
            ptr1->~T();
            ptr1++;
        }
    }
}

#endif 