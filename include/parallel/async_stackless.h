//
// Created by yuchengye on 2021/11/17.
//

#ifndef JOBLIB_ASYNC_STACKLESS_H
#define JOBLIB_ASYNC_STACKLESS_H
#include "thread"
#include "string"
#include "chrono"
#include "cstdio"
#include "iostream"
#include "algorithm"
#include "future"
#include "functional"
#include "iterator"
#include <cstring>
#include <vector>
#include <set>
#include <utility>
#include "types.h"
#include "user_stack.hpp"
#define DEFAULT_CORO_STACK_SIZE 1024*256
#define SUSPENDED 0
#define RUNNING 1
#define AWAITING 2
#define DESTROYED 3

#define CO_STKLESS(name,...) \
struct name:public stackless::co{ \
    name():stackless::co(__VA_ARGS__){                 \
        fp=union_cast<void*>(&name::_co_func_);      \
    }                         \

#define CO_DEF(RET_TYPE, ...) \
    using ret_type = RET_TYPE;              \
    void _co_func_ ( __VA_ARGS__ ){ \
    co_v_status=RUNNING;                        \
    switch(co_v_state){                 \
    case 0:                          \
        _res_=(RET_TYPE*)malloc(sizeof(RET_TYPE));             \

#define CO_DEF_END } \
}                    \
};\

#define CO_AWAIT(new_co, ...) _co_await<new_co::ret_type>(new new_co(), ##__VA_ARGS__ )


#define CO_YIELD(ret)        \
co_v_state=__COUNTER__+2;    \
*reinterpret_cast<decltype(ret)*>(_res_)=ret; \
co_v_status=SUSPENDED;                             \
return;                 \
case __COUNTER__+1:;         \

#define CO_RET(ret)     \
co_v_state=0;           \
*reinterpret_cast<decltype(ret)*>(_res_)=ret; \
co_v_status=DESTROYED;               \
return;                 \

/**
 * 获取类方法指针
 * @tparam dst_type
 * @tparam src_type
 * @param src
 * @return
 */
template<typename dst_type,typename src_type>
dst_type union_cast(src_type src)
{
    union{
        src_type s;
        dst_type d;
    }u;
    u.s = src;
    return u.d;
}

namespace stackless {

//    template<typename ret_type>
    struct co{
        stackless::co *co_v_parent;
        std::set<stackless::co*> co_v_childs;    // TODO 考虑链表实现
        bool isRootCo;
        uint8_t co_v_status;
        uint32_t co_v_state;
        uint8_t co_v_state_cnt;
        void* fp;
    public:
        void* _res_;

        co():co_v_status(SUSPENDED), co_v_state(0), co_v_state_cnt(0),
             isRootCo(false), co_v_parent(nullptr){}

        explicit co(stackless::co* , bool);

        void mount_on(stackless::co* _parent);

        void unmount_from();

        void mount(stackless::co *_child);

        void unmount(stackless::co *_child);

        template<class T>
        void await(stackless::co *new_co);

        template<class... Arg>
        void run_once(Arg... args);

        template<class T>
        T get();

        void clean();
    };

    /**
     * @brief 同一async_task的coroutine保证具有正确的关系
     */
    class async_task{
    private:
        char *task_name;
        uint8_t status;
        friend class event_loop_s;


    public:
        stackless::co* root_co;
        stackless::co* running_co;
        user_stack* stack;

        template<class... Arg>
        void run(Arg... args);

        explicit async_task(stackless::co *co);

        async_task(stackless::co *co, const char *name);

        stackless::co* get_running_co();

        bool switch_to(stackless::co* dst_co);
    };

    class event_loop_s{
    public:
        uint8_t status;
        std::vector<async_task*> tasks;
        uint8_t running_task_idx;

        event_loop_s(){}

        event_loop_s(async_task* task){tasks.push_back(task);}

        event_loop_s(std::vector<async_task*>::iterator begin, std::vector<async_task*>::iterator end){
            std::copy(begin, end, tasks.begin());
        }

        async_task* get_running_task(){
            if(status==RUNNING){
                return tasks[running_task_idx];
            }
            std::cerr<<"There is no running task."<<std::endl;
            return nullptr;
        };
        bool run_until_complete(){

            return false;
        }
    };

    thread_local co* curr_running_co=nullptr;
    thread_local event_loop_s* running_loop=nullptr;

    event_loop_s* get_running_loop();

    template<typename ret_type, typename... Arg>
    ret_type _co_await(co* new_co, Arg... args);

    void _co_await(co* new_co);

    void co_end();

    void coro_entry(std::function<void()> *f);

    void co_ret();

    void loop_local_scheduler();
}

#include "async_stackless_impl.h"

#endif //JOBLIB_ASYNC_STACKLESS_H
