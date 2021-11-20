#include "alloc.hpp"
#include <iostream>
#include <initializer_list>
#include <utility>
#include <algorithm>
#include <string>

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

    template<typename... Args>
        void emplace_back(Args&&...) noexcept;

    void push_back(const T&);
    void push_back(T&&) noexcept;
    void pop_back();

    T* begin(){ return elements_;}
    T* end() { return first_free_;}
    const T* begin() const {return elements_;}
    const T*end() const {return first_free_;}

    void reserve(size_t n);
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
    alloc alloc_;
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
    T* head = (T*)alloc_.allocate(newsize * sizeof(T));
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
    T* head = (T*)alloc_.allocate((last - first) * sizeof(T));
    return {head, uninitialized_copy(first, last, head)};
}

template<typename T>
void vec<T>::reserve(size_t n)
{
    if (n > capacity()) {
        reallocate();
    }
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
void vec<T>::push_back(T&& t) noexcept
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

template<typename T>
template<typename... Args>
void vec<T>::emplace_back(Args&&... args) noexcept
{
        check_n_alloc();

        alloc_.construct(first_free_++, std::forward<Args>(args)...);
}

class Test
{
public:
    Test() {}

    Test(int aa, std::string s)
        : a(aa)
        , name(s)
    { std::cout << "Test(aa, s)!\n"; }

    Test(const Test& t)
        : a(t.a)
        , name(t.name)
    { std::cout << "const Test&\n"; }

    Test(Test&& t)
        : a(t.a)
        , name(t.name)
    { std::cout << "Test&&!\n";}

    ~Test()
    { std::cout << "~Test()!\n"; }
private:
    int a;
    std::string name;
};


int main()
{

    std::cout << "push_back:------------------------------------------------\n";
    vec<Test> vs;
    vs.reserve(2);
    vs.push_back(Test(1, "czz"));
    std::cout << "--------------------------" << std::endl << std::endl;



    vec<std::string> c;
    c.push_back("czz");
    std::cout<<c.size()<<"  "<<c.capacity()<<std::endl;


    c.push_back("llr");
    c.push_back("czzxllr");
    c.pop_back();
    for (auto i = c.begin();i != c.end(); i++)
        std::cout << *i << " " ; std::cout<<"\n";
    std::cout<<c.size()<<"  "<<c.capacity()<<std::endl;
    c.reserve(3);

    return 0;
}
