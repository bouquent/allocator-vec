#include <iostream>
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <string>

void* operator new(size_t t)
{
    auto mem = (void*)malloc(t);
    if (nullptr == mem) {
        return nullptr;
    }
    return mem;
}

void operator delete(void *p)
{
    if (p == nullptr) {
        return ;
    }
    free(p);
}


void* operator new[] (size_t t)
{
    auto mem = (void*)malloc(t);
    if (nullptr == mem) {
        return nullptr;
    }
    return mem;
}

void operator delete[](void *p)
{
    if (p == nullptr) {
        return ;
    }
    free(p);
}

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

    template<class Ty1, class Ty2>
    void construct (Ty1 *ptr, Ty2&& value)
    {
        ::new(ptr) Ty2(std::move(value)); //调用Ty2类型的移动构造函数
    }
    /* //可以代替const Ty2& 和 Ty2&& 两个重载函数的方法
    * template<class Ty1, class Ty2>
    * void construct (Ty1 *ptr, Ty2&& value)
    * {
    *   ::new(ptr) Ty2(std::forward(value));
    * }
    */
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


template<class T>
class vec
{
public:
    vec();
    vec(const vec&);
    vec(vec&&) noexcept;
    vec(const std::initializer_list<T>& li);
    vec& operator= (const vec&);
    vec& operator= (vec&&) noexcept;
    vec& operator= (const std::initializer_list<T>& li);
    ~vec();

    void push_back(const T&);
    void push_back(T&&);
    void pop_back();

    T* begin(){ return elements_;}
    T* end() { return first_free_;}
    const T* begin() const {return elements_;}
    const *end() const {return first_free_;}

    size_t size() { return first_free_ - elements_; }
    size_t capacity() { return cap_ - elements_; }
private:
    void check_n_alloc();
    void reallocate();
    std::pair<T*, T*> alloc_n_copy(const T *first, const T *last);
    void free();

    T *elements_;
    T *first_free_;
    T *cap_;
    allocator<T> alloc_;
};

template<class T>
void vec<T>::check_n_alloc()
{
    if (size() == capacity()) {
        /*需要扩容*/
        reallocate();
    }
}

template<class T>
void vec<T>::free()
{
    if (elements_ != nullptr) {
        for (auto iter = elements_; iter != first_free_; ++iter) {
            alloc_.destroy(*&iter);
        }
        alloc_.deallocate(elements_, sizeof(T) * (cap_ - elements_));
    }
}

template<class T>
void vec<T>::reallocate()
{
    auto newsize = size() * 2 + 1;

    /*重新寻找到一块更大的内存*/
    auto head = alloc_.allocate(newsize);
    auto tmphead = head;
    auto tmpelem = elements_;
    for (size_t i = 0; i < size(); ++i) {
        alloc_.construct(tmphead++, *tmpelem++);
    }

    /*释放掉之前的空间*/
    free();

    elements_ = head;
    /*此时tmphead的位置就是尾结点的后一个位置*/
    first_free_ = tmphead;
    cap_ = elements_ + newsize;
}

template<class T>
std::pair<T*, T*> vec<T>::alloc_n_copy(const T* first, const T* last)
{
    auto head = alloc_.allocate(last - first);
    return {head, uninitialized_copy(first, last, head)};
}



template<class T>
vec<T>::vec()
    : elements_(nullptr)
    , first_free_(nullptr)
    , cap_(nullptr)
{}

template<class T>
vec<T>::vec(const vec &v)
{
    auto data = alloc_n_copy(v.begin(), v.end());

    elements_ = data.first;
    first_free_ = data.second;
    cap_ = data.second;
}

template<class T>
vec<T>::vec(vec&& v) noexcept
{
    elements_ = v.elements_;
    first_free_ = v.first_free_;
    cap_ = v.cap_;

    //将右值引用变量指针全部置空
    v.elements_ = v.first_free_ = v.cap_ = nullptr;
}

template<class T>
vec<T>::vec(const std::initializer_list<T>& li)
{
    auto data = alloc_n_copy(li.begin(), li.end());
    elements_ = data.first;
    first_free_ = data.second;
    cap_ = data.second;
}

template<class T>
vec<T>& vec<T>::operator= (const vec& v)
{
    if (this == &this) return *this;
    /*释放已有资源，准备赋值*/
    free();

    auto data = alloc_n_copy(v.begin(), v.end());
    elements_ = data.first;
    first_free_ = data.second;
    cap_ = data.second;
    return *this;
}

template<class T>
vec<T>& vec<T>::operator= (vec&& v) noexcept
{
    elements_ = v.elements_;
    first_free_ = v.first_free_;
    cap_ = v.cap_;

    //将右值引用变量指针全部置空
    v.elements_ = v.first_free_ = v.cap_ = nullptr;
    return *this;
}

template<class T>
vec<T>& vec<T>::operator= (const std::initializer_list<T>& li)
{
    auto data = alloc_n_copy(li.begin(), li.end());
    elements_ = data.first;
    first_free_ = data.second;
    cap_ = data.second;
    return *this;
}

template<class T>
vec<T>::~vec()
{
    free();
}

template<class T>
void vec<T>::push_back(const T& t)
{
    /*检查是否已满，如果已满则分配新的更大的内存空间*/
    check_n_alloc();

    alloc_.construct(first_free_++, t);
}


template<class T>
void vec<T>::push_back(T&& t)
{
   /*检查是否已满，如果已满则分配新的更大的内存空间*/
    check_n_alloc();

    alloc_.construct(first_free_++, std::move(t));
}


template<class T>
void vec<T>::pop_back()
{
    if (size() > 0) {
        alloc_.destroy(--first_free_);
    }
}


int main()
{
    //测试
    vec<std::string> c;
    std::cout << "go ahead\n";
    c.push_back("czz");
    for (auto i = c.begin();i != c.end(); i++)
        std::cout << *i << " " ; std::cout<<"\n";
    std::cout<<c.size()<<"  "<<c.capacity()<<std::endl;


    c.push_back("llr");
    c.push_back("czzllr");
    c.push_back("czzxllr");
    for (auto i = c.begin();i != c.end(); i++)
        std::cout << *i << " " ; std::cout<<"\n";
    std::cout<<c.size()<<"  "<<c.capacity()<<std::endl;

    c.pop_back();
    for (auto i = c.begin();i != c.end(); i++)
        std::cout << *i << " " ; std::cout<<"\n";
    std::cout<<c.size()<<"  "<<c.capacity()<<std::endl;

    return 0;
}
