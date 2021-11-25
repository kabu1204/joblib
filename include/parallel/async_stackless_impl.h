//
// Created by yuchengye on 2021/11/17.
//

#include "parallel/async_stackless.h"
using namespace stackless;


stackless::co::co(stackless::co * _parent, bool _isRootCo=false):co_v_status(SUSPENDED), co_v_state(0), co_v_state_cnt(0),
isRootCo(_isRootCo){
        if(!isRootCo) co_v_parent=_parent;
}

void stackless::co::mount_on(stackless::co* _parent){
    /*
     * call mount_on() to set relation between parent and child
     */
    if(isRootCo){
        std::cerr<<"Root coroutine cannot be awaited."<<std::endl;
        return;
    }
    co_v_parent = _parent;
    co_v_parent->mount(this);
}

void stackless::co::unmount_from(){
    /*
     * call unmount_from() to reset relation between parent and child
     */
    if(isRootCo){
        std::cerr<<"Root coroutine has no parent."<<std::endl;
        return;
    }
    co_v_parent->unmount(this);
    co_v_parent=nullptr;
}

void stackless::co::mount(stackless::co *_child){
    // TODO: 注意判重
    co_v_childs.insert(_child);
}

void stackless::co::unmount(stackless::co *_child){
    co_v_childs.erase(_child);
}

template<class T>
void stackless::co::await(stackless::co *new_co){
    new_co->mount_on(this);
    // call scheduler
}

template<class... Arg>
void stackless::co::run_once(Arg... args){
    __asm inline volatile(
    "callq *%[cofunc]"
    ::[cofunc]"a"(fp)
    );
}

void stackless::co::clean(){
    if(!co_v_childs.empty()){
        std::cerr<<"To reset an internal coroutine node is NOT ALLOWED.\n"<<std::endl;
        return;
    }
    co_v_state=0;
    if(!isRootCo) unmount_from();
}

template<class T>
T co::get() {
    return *(T*)_res_;
}

void co::run_once_stdfunc() {
    _f();
}

template<>
void co::get() {/*do nothing*/}

template<class RET_T, class RECV_T>
RET_T generator_s<RET_T, RECV_T>::next() {
    _f();
    return *(RET_T*)_res_;
}

template<class RET_T, class RECV_T>
RET_T generator_s<RET_T, RECV_T>::send(RET_T content){
    *reinterpret_cast<RECV_T*>(_recv_)=content;
    _f();
    return *(RET_T*)_res_;
}

template<class RET_T, class RECV_T>
void generator_s<RET_T, RECV_T>::_yield(RET_T ret){
    *reinterpret_cast<RET_T*>(_res_)=ret;
}


// TODO: 返回值的存储，指针类型的转换，调用派生类函数
GEN_STKLESS(co_example, int, int)
    //1: Declare local variables you need
    int a,b,i;
    //2: Declare function and implement it like this
    GEN_DEF(int c)  // function ProtoType
        a=c;            // DO NOT USE parameters directly, it will not be saved; save it in a local variable;
        for(i=0;i<=10;++i) {
            CO_YIELD(i,b);       // yield wherever you need to
        }
        CO_RET(11);     // return value
DEF_END;         // be sure to DEF_END at end


stackless::async_task::async_task(stackless::co *co) {
    if(!co->isRootCo){
        std::cerr<<"Async task must be built from a root coroutine."<<std::endl;
        root_co=nullptr;
        return;
    }
    root_co = co;

    stack = new user_stack(DEFAULT_CORO_STACK_SIZE);
    stack->regs.rdi=(DWORD)&co->_f;
    stack->pushq(stackless::co_entry);
}

stackless::async_task::async_task(stackless::co *co, const char *name){
    if(name){
        size_t len = strlen(name);
        task_name=new char[len];
        strncpy(task_name, name, len);
    }
    if(!co->isRootCo){
        std::cerr<<"Async task must be built from a root coroutine."<<std::endl;
        root_co=nullptr;
        return;
    }
    root_co = co;
}

stackless::co* stackless::async_task::get_running_co(){
    if(status==RUNNING){
        return running_co;
    }
    std::cerr<<"There is no running coroutine."<<std::endl;
    return nullptr;
}

void stackless::async_task::scheduler() {
    // used by co_gather();
}

/**
 * 从running_co切换至dst_co (调用)。该函数只应该由event_loop调用
 * @param new_co
 * @return
 */
bool stackless::async_task::switch_to(stackless::co* dst_co){
    // TODO:多线程调度请加锁
    dst_co->co_v_state=RUNNING;
    running_co->co_v_state=SUSPENDED;
    running_co=dst_co;
    return true;
}

void stackless::async_task::run(){
    status=RUNNING;
    running_co=root_co;
    curr_running_co=running_co;
    root_co->run_once_stdfunc();
}


stackless::event_loop_s* stackless::get_running_loop() {
    if(running_loop && running_loop->status==RUNNING){
        return running_loop;
    }
    std::cerr<<"There is no running event loop in current thread."<<std::endl;
    return nullptr;
}

template<typename ret_type, typename... Arg>
ret_type stackless::_co_await(stackless::co* new_co, Arg... args) {
    // TODO: status setting
    stackless::event_loop_s* loop=stackless::get_running_loop();
    stackless::async_task* task=loop->get_running_task();
    stackless::co* old_co=task->get_running_co();

    new_co->mount_on(old_co);
    task->running_co=new_co;
    old_co->co_v_status=SUSPENDED;
    uint8_t next_task = loop_local_scheduler();
    if(next_task!=loop->running_task_idx){
        task->status=SUSPENDED;
        loop->running_task_idx=next_task;
        switch_user_context(task->stack, loop->tasks[next_task]->stack);
        // switch_to_next_task
    }
    // switched back or not switched at all
    task->status=RUNNING;
    curr_running_co=task->running_co;
    new_co->run_once(args...);
    return new_co->get<ret_type>();
}

void stackless::_co_await(stackless::co* new_co) {
    // TODO: status setting
//    T* _new_co = static_cast<T*>(old_co);
    stackless::event_loop_s* loop=stackless::get_running_loop();
    stackless::async_task* task=loop->get_running_task();
    stackless::co* src_co=task->get_running_co();

    new_co->mount_on(src_co);
    // TODO:此时task已不是running状态，考虑是否有必要设置status和running_co
    // goto scheduler
    task->running_co=new_co;
    uint8_t next_task = loop_local_scheduler();
    if(next_task!=loop->running_task_idx){
        loop->running_task_idx=next_task;
        // switch_to_next_task
    }
    // switched back or not switched at all
    curr_running_co=task->running_co;
}

uint8_t stackless::loop_local_scheduler(){
    //TODO
    stackless::event_loop_s* loop=stackless::get_running_loop();
    uint8_t next_task_idx = (loop->running_task_idx+1)%loop->tasks.size();
    return next_task_idx;
}



async_task* stackless::event_loop_s::get_running_task(){
    if(status==RUNNING){
        return tasks[running_task_idx];
    }
    std::cerr<<"There is no running task."<<std::endl;
    return nullptr;
};

bool stackless::event_loop_s::run_until_complete(){
    status=RUNNING;
    running_loop=this;
    running_task_idx = loop_local_scheduler();
    switch_user_context(stack, tasks[running_task_idx]->stack);
    return true;
}
