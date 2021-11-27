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
    dprint("");
    _f();
}

template<>
void co::get() {/*do nothing*/}

template<class RET_T, class RECV_T>
RET_T generator_s<RET_T, RECV_T>::next() {
    _f();
    if(stopped){
        dprint("");
        throw std::runtime_error("StopIteration");
    }
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

void tt(){
    dprint("");
}

stackless::async_task::async_task(stackless::co *co) {
    if(!co->isRootCo){
        std::cerr<<"Async task must be built from a root coroutine."<<std::endl;
        root_co=nullptr;
        return;
    }
    root_co = co;

    dprint("Construct async_task\n");
    stack = new user_stack(DEFAULT_CORO_STACK_SIZE);
    _run = std::bind(&stackless::async_task::run,this);
    _prun=&_run;
    stack->regs.rdi=(DWORD)(_prun);
    stack->pushq<DWORD>(0xdeadbeef);    // keep 16-bytes aligned
    stack->pushq<void(std::function<void()>*)>(stackless::co_entry);
//    stack->regs.rdi=(DWORD)tt;
//    stack->pushq<DWORD>(0xdeadbeef);    // keep 16-bytes aligned
//    stack->pushq<void()>(stackless::co_entry);
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
    dprint("in run\n");
    status=RUNNING;
    running_co=root_co;
    curr_running_co=running_co;
    root_co->run_once();
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
    async_task* next_task = loop_local_scheduler(task);
    if(next_task!=task){
        switch_user_context(task->stack, next_task->stack);
        // switch_to_next_task
    }
    // switched back or not switched at all
    curr_running_co=task->running_co;
    new_co->run_once(args...);
    return new_co->get<ret_type>();
}

/**
 * enqueue the old_task -> select next_task to run -> dequeue the next_task -> modify the status
 * @param old_task pointer of old_task
 * @return pointer of next_task
 */
async_task* stackless::loop_local_scheduler(async_task* old_task){
    stackless::event_loop_s* loop=stackless::get_running_loop();
    if(old_task){
        old_task->status=SUSPENDED;
        loop->tasks.push(old_task);
    }
    dprint("size of tasks:%lu\n",loop->tasks.size());
    async_task* next_task = loop->tasks.front();
    loop->tasks.pop();
    loop->running_task=next_task;
    next_task->status=RUNNING;
    return next_task;
}


async_task* stackless::event_loop_s::get_running_task(){
    if(status==RUNNING){
        return running_task;
    }
    std::cerr<<"There is no running task."<<std::endl;
    return nullptr;
};

bool stackless::event_loop_s::run_until_complete(){
    status=RUNNING;
    running_loop=this;
    running_task = loop_local_scheduler(nullptr);
    do{
        dprint("switching to task\n");
        switch_user_context(stack, running_task->stack);
        dprint("switched back from task\n");
    } while(status != DESTROYED);
    return true;
}

void stackless::event_loop_s::notify_running_task_end(){
    dprint("task end.\n");
    async_task* old_task=running_task;
    old_task->status=DESTROYED;
    if(!tasks.empty()){
        loop_local_scheduler(nullptr);
    }
    else{
        status=DESTROYED;
    }
    switch_user_context(old_task->stack, stack);
}

/**
 * Entry of a wrapped coroutine. Also the entry of an async_task.
 * @param f function to run
 */
void stackless::co_entry(std::function<void()> *f) {
    dprint("co_entry\n");
    (*f)();
    dprint("back to co_entry\n");
    event_loop_s* loop=get_running_loop();
    loop->notify_running_task_end();
}

void stackless::co_entry(FP_CALL_BACK f) {
    dprint("co_entry\n");
    f();
    dprint("back to co_entry\n");
    event_loop_s* loop=get_running_loop();
    loop->notify_running_task_end();
}

void stackless::co_entry() {
//    __asm inline volatile(
//            "subq $8, %rsp\n\t"
//            );
    dprint("co_entry\n");
    event_loop_s* loop=get_running_loop();
    loop->running_task->run();
    dprint("back to co_entry\n");
    loop->notify_running_task_end();
}
