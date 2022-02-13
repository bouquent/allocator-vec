#ifndef ALLOC_H
#define ALLOC_H
#include <stdlib.h>
#include <stdio.h>

enum {ALIGN = 8};
enum {MAX_BYTES = 128};
enum {FreeListNum = MAX_BYTES / ALIGN};


//每个空闲块都有一个obj的指针，指向下一个空闲块，但是当这个空闲块被使用后。
//这个指针所占用的空间会被用于存储数据，所以不会造成一点空间的浪费
union obj{
    union obj* free_list_link;
    char client_data[1];
};

class alloc
{
public:
    static void* allocate(size_t n);
    static void deallocate(void*, size_t n);
    static void* reallocate(size_t n);
private:
    //为了将不够的部分向上调整需要先+7   为了将低位上的数划去直接 &~7
    static size_t round_up(size_t bytes) {
        return ((bytes + ALIGN - 1) & ~(ALIGN - 1));
    }
    static size_t freelist_index(size_t bytes) {
        return (bytes + ALIGN - 1) / ALIGN - 1;
        
    }
    static void* refill(size_t n);
    static char* chunk_alloc(size_t size, int &nobjs);

private:
    static char* start_free;    //内存池起始和结束地址
    static char* end_free;
    static size_t heap_size;    //向堆中申请的内存总大小

    static obj* volatile free_list[FreeListNum];    //对应不同内存大小的空闲链表
};

char* alloc::start_free = 0;
char* alloc::end_free = 0;
size_t alloc::heap_size = 0;

obj* volatile alloc::free_list[FreeListNum] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

void* alloc::allocate(size_t n) 
{
    //大于128直接调用malloc
    if (n > MAX_BYTES) {  
        return ::malloc(n); 
    }

    //找到合适的空闲链表，同时获取空闲链表空闲区域的头结点
    obj* volatile* my_free_list = free_list + freelist_index(n);
    obj* result = *my_free_list;
    
    if (result == nullptr) {
        //如果为空，没有空闲区块,那么从内存池挖取内存并分割
        void *r = refill(round_up(n));
        return r;
    }

    //修改这个空闲链表的头结点，让它指向空闲区块的头结点
    *my_free_list = result->free_list_link;
    return result;
}

void alloc::deallocate(void *ptr, size_t n) 
{
    if (n > MAX_BYTES) {
        return ::free(ptr);
    }
    obj* volatile* my_free_list;
    obj* q = (obj*)ptr;   

    //将归还的内存转化成空闲块的状态（就是让他指向当前的空闲块头部，并且让它当新的空闲块头部）
    //并维护空闲链表
    my_free_list = free_list + freelist_index(n);
    q->free_list_link = *my_free_list;
    *my_free_list = q;
}

void* alloc::refill(size_t n)
{
    int nobjs = 20;
    char* chunk = chunk_alloc(n, nobjs);
    if (nobjs == 1) {
        //只获得了一个区块，直接返回
        //此时不需要维护空闲链表，因为这个chunk一定是当前空闲链表的最后一块空闲块，它的后面没有空闲块了
        return chunk;
    }

    //将获取的大空闲块使用指针一块块穿起来
    obj* volatile* my_free_list;
    my_free_list = free_list + freelist_index(n);
    obj* result = (obj*)chunk;
    //首块空闲内存块的地址，第一块是给用户的
    *my_free_list = result + n;


    obj* current_free_list = *my_free_list;
    obj* next_free_list = current_free_list + n;
    //n块内存，一块分配给客户，剩下n-1块只需要连接n-2次就好了
    for (int i = 0; i < nobjs - 2; ++i) {
        current_free_list->free_list_link = next_free_list;
        current_free_list = next_free_list;
        next_free_list += n;
    }
    //最后一个空闲块不指向任何地方
    current_free_list->free_list_link = nullptr;
    return result;
}

char* alloc::chunk_alloc(size_t bytes, int& nobjs)
{
    char *result;
    //每次挖取20块内存
    size_t total_bytes = bytes * nobjs;
    //查看内存池容量
    size_t bytes_left = end_free - start_free;  


    if (bytes_left > total_bytes) {
        //剩余量可以满足20块bytes大小内存的请求
        result = start_free;
        start_free += total_bytes;
        return result;
    } else if (bytes_left >= bytes) {
        //虽然不能分割20个，但是可以提供几个给上层
        nobjs = bytes_left / bytes;
        total_bytes = bytes * nobjs;
        result = start_free;
        start_free += total_bytes;
        return result;
    } else {
        //一个都没办法满足了，只能malloc申请更多的内存，每次malloc的大小为20*2*sizeof(bytes)
        //或者从其他更大的空闲链表上借一块内存
        if (bytes_left > 0) {
            //先之前大于0的剩余量挂到相应的一个链表中管理
            obj* volatile* my_free_list = free_list + freelist_index(bytes_left);
            ((obj*)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj*)start_free;
        }

        size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);
        printf("bytes to get size: %ld\n", bytes_to_get);
        start_free = (char*)::malloc(bytes_to_get);
        if (nullptr == start_free) {
            //malloc申请失败
            obj* volatile* my_free_list;
            obj* p;
            for (int i = bytes; i <= MAX_BYTES; i += ALIGN) {
                my_free_list = free_list + freelist_index(i);
                p = *my_free_list;
                if (nullptr != p) {
                    //在更大的空闲链表上借一块内存
                    *my_free_list = p->free_list_link;
                    start_free = (char*)p;
                    end_free = start_free + i;
                    
                    return (chunk_alloc(bytes, nobjs));
                }
            }

            end_free = nullptr;
            //使内存的情况得到改善 或者 抛出异常
            start_free = (char*)::operator new(bytes_to_get);
        } 

        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return (chunk_alloc(bytes, nobjs)); //重新分配一次
    }
}

#endif 