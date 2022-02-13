#ifndef MYVECTOR_H
#define MYVECTOR_H
#include <memory>
#include "myalloc.h"
#include "allocator.h"

template<typename T, typename Alloc = myalloc<T>>
class vector
{
public:
    typedef T value_type;
    typedef T& reference;
    typedef T* pointer;
    typedef size_t type_size;
    typedef ptrdiff_t difference_size;

public:
    vector();
    vector(size_t n);
    vector(const vector& v);
    vector& operator=(const vector& v);
    vector(vector&& v);

    void push_back(const T& value);
    void push_back(T&& value);
    template<typename... Args>
    void emplace_back(Args... args);
    void pop_back();


    size_t size() const { return last_ - first_; }
    size_t capacity() const { return cap_ - first_; }
    T* begin() { return first_;}
    const T* begin() const { return first_; }
    T* end() { return last_; }
    const T* end() const { return last_; }
    bool empty() const { return first_ == last_;}
private:
    T* reallocate();
    void check_n_alloc();
    void free();

private:
    Alloc alloc_;
    T* first_;
    T* last_;
    T* cap_;
};

template<typename T, typename Alloc>
vector<T, Alloc>::vector()
{
    first_ = nullptr;
    last_ = nullptr;
    cap_ = nullptr;
}

template<typename T, typename Alloc>
vector<T, Alloc>::vector(size_t n) 
{
    if (n < 0) {
        throw "vector size need bigger than 0";
    }
    first_ = alloc_.allocate(n);
    last_ = first_;
    cap_ = first_ + n;
}

template<typename T, typename Alloc>
vector<T, Alloc>::vector(const vector& v)
{
    first_ = alloc_.allocate(v.capacity());
    last_ = std::uninitialized_copy(v.begin(), v.end(), first_);
    cap_ = first_ + v.capacity();
}

template<typename T, typename Alloc>
vector<T, Alloc>& vector<T, Alloc>::operator=(const vector& v)
{
    if (this == &v) {
        return *this;
    }
    T* newhead = alloc_.allocate(v.capacity());
    std::uninitialized_copy(v.begin(), v.end(), newhead);

    free();         //释放原有内存
    first_ = newhead;
    last_ = first_ + v.size();
    cap_ = first_ + v.capcaity();
}

template<typename T, typename Alloc>
vector<T, Alloc>::vector(vector&& v) 
{
    first_ = v.first_;
    last_ = v.last_;
    cap_ = v.cap_;
    v.first_ = nullptr;
    v.last_ = nullptr;
    v.cap_ = nullptr;
}

template<typename T, typename Alloc>
void vector<T, Alloc>::push_back(const T& value)
{
    check_n_alloc();
    alloc_.construct(last_++, value);
}

template<typename T, typename Alloc>
void vector<T, Alloc>::push_back(T&& value)
{
    check_n_alloc();
    alloc_.construct(last_++, std::move(value));
}

template<typename T, typename Alloc>
template<typename... Args>
void vector<T, Alloc>::emplace_back(Args... args)
{
    check_n_alloc();
    alloc_.construct(last_++, std::forward<Args>(args)...);
}

template<typename T, typename Alloc>
void vector<T, Alloc>::pop_back()
{
    if (this->empty()) {
        throw "the vector is empty, pop_back is failure";
    }
    alloc_.destroy(--last_);
}

template<typename T, typename Alloc>
void vector<T, Alloc>::free()
{
    if (begin() != nullptr) {
        alloc_.destroy(begin(), end());
        alloc_.deallocate(begin(), capacity());
    }
}

template<typename T, typename Alloc>
void vector<T, Alloc>::check_n_alloc()
{
    if (last_ == cap_) {
        reallocate();
    }
}

template<typename T, typename Alloc>
T* vector<T, Alloc>::reallocate()
{
    int size = this->size();
    size_t newsize = (size == 0 ? 1 : size * 2);
    T* newhead = alloc_.allocate(newsize);
    std::uninitialized_copy(begin(), end(), newhead);

    free();
    first_ = newhead;
    last_ = newhead + size;
    cap_ = newhead + newsize;
    return first_;
}

#endif 