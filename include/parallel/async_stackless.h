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
    }                        \
    template<class... Arg>                          \
    void operator() (Arg... args){                                  \
        _f=std::bind(&name::_co_func_, this, args...); \
        _fp=&_f;                         \
    }\


#define CO_DEF(RET_TYPE, ...) \
    using ret_type = RET_TYPE;              \
    void _co_func_ ( __VA_ARGS__ ){ \
    co_v_status=RUNNING;                        \
    switch(co_v_state){                 \
    case 0:                          \
        _res_=(RET_TYPE*)malloc(sizeof(RET_TYPE));             \


#define GEN_STKLESS(name, RET_TYPE, RECV_TYPE,...) \
struct name:public stackless::generator_s<RET_TYPE, RECV_TYPE>{ \
    using ret_type = RET_TYPE;  \
    using recv_type = RECV_TYPE;                  \
    name(__VA_ARGS__):stackless::generator_s<RET_TYPE, RECV_TYPE>(__VA_ARGS__){  \
        fp=union_cast<void*>(&name::_co_func_);    \
    }                                               \
    template<class... Arg>                          \
    void operator() (Arg... args){                                  \
        _f=std::bind(&name::_co_func_, this, args...);      \
    }

#define GEN_DEF(...) \
    void _co_func_ ( __VA_ARGS__ ){ \
    co_v_status=RUNNING;                        \
    switch(co_v_state){                 \
    case 0:                          \
        _res_=(ret_type*)malloc(sizeof(ret_type)); \


#define DEF_END } \
}                 \
};\

#define GEN_END } \
stopped=true;     \
}                 \
};\

#define CO_AWAIT(new_co, ...) _co_await<new_co::ret_type>(new new_co(), ##__VA_ARGS__ )


#define CO_YIELD(ret, recv) \
co_v_state=__COUNTER__+2;    \
_yield(ret); \
co_v_status=SUSPENDED;       \
return;                     \
case __COUNTER__+1:;        \
recv=_recv_;

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
        std::function<void()> _f;
        std::function<void()>* _fp;
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

        void run_once_stdfunc();

        template<class T>
        T get();

        void clean();

    };

    template<class RET_T, class RECV_T>
    struct generator_s:public co{
        using ret_type = RET_T;
        using recv_type = RECV_T;
        bool stopped{false};
//        std::function<void()> _f;
    public:
        RECV_T _recv_;

        std::function<void()> wrapper;

        generator_s():co(){
//            _args_ = std::make_tuple(args...);
        }

        RET_T next();

        RET_T send(RET_T content);

        void _yield(RET_T ret);
    };

    /**
     * @brief 同一async_task的coroutine保证具有正确的关系
     */
    class async_task{
    private:
        char *task_name;
        friend class event_loop_s;

    public:
        stackless::co* root_co;
        stackless::co* running_co;
        user_stack* stack;
        uint8_t status;
        std::function<void()> _run;
        std::function<void()>* _prun;

        void run();

        explicit async_task(stackless::co *co);

        async_task(stackless::co *co, const char *name);

        stackless::co* get_running_co();

        void scheduler();

        bool switch_to(stackless::co* dst_co);
    };

    class event_loop_s{
    public:
        uint8_t status{SUSPENDED};
        std::queue<async_task*> tasks;
        uint8_t running_task_idx{0};
        async_task* running_task;
        user_stack* stack;

        event_loop_s(){stack=new user_stack(0);}

        event_loop_s(async_task* task){tasks.push(task);stack=new user_stack(0);}

        event_loop_s(std::vector<async_task*>::iterator begin, std::vector<async_task*>::iterator end){
            for(auto it=begin;it!=end;++it) tasks.push(*it);
            stack=new user_stack(0);
        }

        async_task* get_running_task();

        bool run_until_complete();

        void notify_running_task_end();
    };

    thread_local co* curr_running_co=nullptr;
    thread_local event_loop_s* running_loop=nullptr;

    event_loop_s* get_running_loop();

    template<typename ret_type, typename... Arg>
    ret_type _co_await(co* new_co, Arg... args);

    void _co_await(co* new_co);

    void _co_yield(co* new_co);

    void co_end();

    void coro_entry(std::function<void()> *f);

    void co_ret();

    async_task* loop_local_scheduler(async_task* old_task=nullptr);

    void co_entry(std::function<void()>* f);

    typedef void (*FP_CALL_BACK)();
    void co_entry(FP_CALL_BACK f);

    void co_entry();
}
#include "async_stackless_impl.h"
#endif //JOBLIB_ASYNC_STACKLESS_H