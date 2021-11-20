#ifndef MYCONSTRUCT_HPP
#define MYCONSTRUCT_HPP
#include <new>

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


#endif