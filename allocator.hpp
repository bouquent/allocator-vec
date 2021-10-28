#include <iostream>
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <string>

namespace myconstruct {
    template<class Ty1>
    void construct(Ty1 *ptr)
    {
        /*在ptr的地址使用默认构造函数构造Ty1类型变量*/
        ::new(ptr) Ty1();
    }

    template<class Ty1, class Ty2>
    void construct(Ty1 *ptr, const Ty2& value)
    {
        ::new(ptr) Ty2(value);
    }

    template<class Ty1, class... Args>
    void construct (Ty1 *ptr, Args&&... args)
    {
        ::new(ptr) Ty1(std::forward<Args>(args)...);
    }

    template<class Ty1>
    void destroy(Ty1 *ptr)
    {
        if (ptr == nullptr) return ;
        ptr->~Ty1();
    }
    template<class Ty1, class Ty2>
    void destroy(Ty1 *first, Ty2 *last)
    {
        for (auto iter = first; iter != last; iter++)
            destroy(*&iter);
    }
};

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
    //std::cout << "allocator::construct(T*, const T&)!\n";
    myconstruct::construct(ptr, value);
}

template<class T>
void allocator<T>::construct(T* ptr, T&& value)  //这里的value是右值引用，但右值引用其实是左值
{
    //std::cout << "allocator::construct(T*, T&&)!\n";
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
