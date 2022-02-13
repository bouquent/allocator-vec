#include "allocator.h"
#include "myalloc.h"
#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
    std::vector<int, allocator<int>> vec;
    for (int i = 0; i < 10; ++i) vec.push_back(i);
    vec.pop_back();
    std::for_each(vec.begin(), vec.end(), [](int a){ std::cout << a << "  "; });
    std::cout << std::endl;

    std::vector<int, myalloc<int>> vec;
    for (int i = 0; i < 10; ++i) vec.push_back(i);
    vec.pop_back();
    std::for_each(vec.begin(), vec.end(), [](int a){ std::cout << a << "  "; });
    std::cout << std::endl;
    return 0;
}