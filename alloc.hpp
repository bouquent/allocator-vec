#ifndef ALLOC_HPP
#define ALLOC_HPP
#include "myconstruct.hpp"
#include <iostream>

using std::size_t;

union obj{
    union obj* free_list_link;
    char client_data[1];  //存储本块的首地址
};

//上调边界
enum {ALIGN = 8};
enum {MAX_BYTES = 128};
/*Free lists个数*/
enum {EFreeListsNumber = MAX_BYTES / ALIGN};


/*如果内存超过128bytes,直接调用std::malloc*/
class alloc
{
private:
    /*将字节向上调整至8的倍数*/
    static size_t round_up(size_t n)
    {
        return (((n) + (size_t)ALIGN - 1) & ~((size_t) ALIGN - 1));
    }
    /*根据申请内存的大小获取对应的freelist的下标*/
    static size_t freelist_index(size_t n)
    {
        return (((n) + (size_t)ALIGN - 1) / (size_t) ALIGN - 1);
    }

private:
    static char* start_free;  /*内存池起始位置*/
    static char* end_free;    /*内存池结束位置*/
    static size_t heap_size;  /*申请heap空间值总大小*/

    static obj* free_list[EFreeListsNumber];  /*自由链表*/
private:
    static size_t M_freelist_index(size_t bytes);
    static void* refill(size_t n);
    static char* chunk_alloc(size_t size, int &nobjs);

public:
    static void* allocate(size_t n);
    static void deallocate(void *p, size_t n);
    static void* reallocate(void *p, size_t old_size, size_t new_size);
    template<class T>
    static void destroy(T* ptr) {
        myconstruct::destroy(ptr);
    }
    template<class T, class... Args>
    static void construct(T* ptr, Args&&... args) {
        myconstruct::construct(ptr, std::forward<Args>(args)...);
    }

};


/*静态成员初始化*/
char* alloc::start_free = 0;
char* alloc::end_free = 0;
size_t alloc::heap_size = 0;
obj* alloc::free_list[EFreeListsNumber] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
};



void* alloc::allocate(size_t n)
{
    std::cout << "allocate bytes : " << n << std::endl;
    obj **my_free_list;
    obj *result;
    if (n > (size_t) MAX_BYTES) {
        std::cout << "直接通过malloc申请内存" << std::endl;
        return ::malloc(n);
    }
    /*找出最合适的freelist*/
    my_free_list = free_list + freelist_index(n);
    result = *my_free_list;

    if (result == 0) {
        /*没有找到可用的free_list,准备填充free_list*/
        void *r = refill(round_up(n));
        return r;
    }

    /*调整freelist，让my_free_list指向下一块内存*/
    *my_free_list = result->free_list_link;
    return result;
}

/*返回一个大小为n的对象，并且再适当会为freelist增加节点
* 这里的n一定已经上调至8的倍数
*/
void *alloc::refill(size_t n)
{
    int nobjs = 20;
    /*尝试取得nobjs个区块作为freelist的增加节点*/
    char *chunk = chunk_alloc(n, nobjs);
    obj **my_free_list;
    obj *result;
    obj *current_obj, *next_obj;
    if (1 == nobjs) {
        /*只获得了一个区块，直接返回给调用者，free_list没有新节点*/
        return chunk;
    }
    //否则调整freelist，纳入新节点
    my_free_list = free_list + freelist_index(n);

    result = (obj*)chunk;
    *my_free_list = next_obj = (obj*)(chunk + n);

    /*将free_list的各节点都穿起来
    *  从1开始，0返回给客户端
    */
    for (int i = 1; ; ++i) {
        current_obj = next_obj;
        next_obj = (obj*)((char*)next_obj + n);
        if (nobjs -1 == i) {
            current_obj->free_list_link = 0;
            break;
        } else {
            current_obj->free_list_link = next_obj;
        }
    }
    return result;
}

char* alloc::chunk_alloc(size_t size, int& nobjs)
{
    char *result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free; //内存池剩余空间

    if (bytes_left > total_bytes) {
        /*剩余量完全满足需求量*/
        result = start_free;
        start_free += total_bytes;
        return (result);
    } else if (bytes_left >= size) {
        /*剩余量不能完全满足需求，但是满足一个以上需求量*/
        nobjs = bytes_left / size;
        total_bytes = nobjs * size;
        result = start_free;
        start_free += total_bytes;
        return result;
    } else {
        /*内存池剩余空间连一个区块大小都无法提供*/
        size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);
        if (bytes_left > 0) {
            /*将残留内存空间编入，处理残留空间*/
            obj **my_free_list = free_list + freelist_index(bytes_left);
            ((obj*) start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj*) start_free;
        }

        start_free = (char*)malloc(bytes_to_get);
        std::cout << "申请了: 2 * " << size << " * " << nobjs << "大小的空间" << std::endl;
        if (0 == start_free) {
            /*malloc申请失败*/
            obj **my_free_list;
            obj *p;
            for (int i = size; i < MAX_BYTES; i += ALIGN) {
                my_free_list = free_list + freelist_index(i);
                p = *my_free_list;
                if (0 != p) {
                    /*这块内存有未用的区块, 割出一块来做内存池*/
                    *my_free_list = p->free_list_link;
                    start_free = (char*)p;
                    end_free = start_free + i;

                    /*找出了一块满足需求的内存*/
                    return (chunk_alloc(size, nobjs));
                }
            }
            end_free = 0;
            return nullptr;
        }

        /*申请成功*/
        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        return (chunk_alloc(size, nobjs));
    }
}

void alloc::deallocate(void *p, size_t n)
{
    obj *q = (obj*)p;
    obj **my_free_list;

    if (n > (size_t) MAX_BYTES) {
        /*大于128，直接使用free*/
        ::free(p);
        return ;
    }
    /*寻找对应的freelist*/
    my_free_list = free_list + freelist_index(n);

    /*调整freelist，回收区块*/
    q->free_list_link = *my_free_list;
    *my_free_list = q;
}


#endif