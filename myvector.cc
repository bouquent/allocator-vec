#include "myvector.h"
#include <iostream>
#include <algorithm>
#include <stdlib.h>


//没有设计迭代器，所以算法模板萃取迭代器的时候，都是使用的特化版本
class Test
{
public:
    Test(int a1 = 0, int b1 = 0) : a(a1), b(b1) {}
    int a;
    int b;
};

int main()
{
    vector<int, myalloc<int>> vec;
    for (int i = 0; i < 19; ++i) {
        vec.push_back(i);
    }
    std::cout << vec.size() << "  " << vec.capacity() << std::endl;
    vec.pop_back();
    std::cout << vec.size() << "  " << vec.capacity() << std::endl;


    vector<int> vec2 = vec;
    std::for_each(vec2.begin(), vec2.end(), [](int a) { std::cout << a << " "; });
    std::cout << std::endl;


    vector<Test> vec3;
    Test t(1, 2);
    vec3.push_back(t);
    vec3.push_back({6, 5});
    for (int i = 0; i < 20; ++i) {
        vec3.push_back({rand(), i});
        std::cout << rand() << " ";
    }
    std::cout << std::endl;

    std::cout << "以第一位排序后:" << std::endl;
    std::sort(vec3.begin(), vec3.end(), [](const Test& t1, const Test& t2){ return t1.a < t2.a; });
    std::for_each(vec3.begin(), vec3.end(),[](const Test& t) { std::cout << t.a << " "; });
    std::cout << std::endl;
    return 0;
}
