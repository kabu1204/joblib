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
#include "types.h"
#define DEFAULT_CORO_STACK_SIZE 1024*256
#define LABEL(cnt) LABEL_##cnt
#define M_CONC(A, B) M_CONC_(A, B)
#define M_CONC_(A, B) A##B
//#define CO_DEF(...) CO_DEF_(int,##__VA_ARGS__)
#define CO_DEF(ret_type, FIRST, ...) \
    void _co_func_ (FIRST, ##__VA_ARGS__ ){ \
    _res_=(ret_type*)malloc(sizeof(ret_type));          \
    switch(co_v_state){                 \
    case 0:;             \

#define CO_DEF_END }}

#define CO_YIELD(ret)        \
co_v_state=__LINE__;    \
*reinterpret_cast<decltype(ret)*>(_res_)=ret;    \
return;                 \
case __LINE__:;         \

#define CO_RET(ret)     \
co_v_state=0;           \
*reinterpret_cast<decltype(ret)*>(_res_)=ret;    \
return;                 \


namespace stackless {
#define SUSPENDED 0
#define RUNNING 1
#define DESTROYED 2

//    template<typename ret_type>
    struct co;

    class async_task;

    class event_loop{
    public:
        uint8_t status;
        std::vector<async_task*> tasks;
        uint8_t running_task_idx;
        event_loop(){}
        event_loop(async_task* task){
            tasks.push_back(task);
        }
        event_loop(std::vector<async_task*>::iterator begin, std::vector<async_task*>::iterator end){
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
    thread_local event_loop* running_loop=nullptr;

    event_loop* get_running_loop();

    template<typename T, typename... Arg>
    void _co_await(co* old_co, co* new_co, Arg... args);

    template<typename T>
    void _co_await(co* old_co, co* new_co);

    void co_yield();

    void co_end();

    void coro_entry(std::function<void()> *f);

    void co_ret();

}
#endif //JOBLIB_ASYNC_STACKLESS_H
