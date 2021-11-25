//
// Created by yuchengye on 2021/11/11.
//

#ifndef JOBLIB_ASYNC_HPP
#define JOBLIB_ASYNC_HPP
#define DEFAULT_CORO_STACK_SIZE 1024*256   // default:256KByte，aligned: 8Byte ~int array of size 32000
#define STACKFUL 1
#define MAX_COROUTINES_PER_LOOP 1024
#define ALLOW_NESTED_ASYNC 0    // allow to create a new event loop in a running event loop
#include "thread"
#include "string"
#include "chrono"
#include "cstdio"
#include "csetjmp"
#include "iostream"
#include "types.h"


int p(int x){std::printf("saving env %d\n",x); return x;};
int p2(int x){std::printf("jumping env %d\n",x); return x;};

//#define yield(a) setjmp(envs[p(running_coro)]); \
//    if(envs_state[running_coro]==0) {           \
//        envs_state[running_coro]=1;             \
//        jmpable_coros.push(running_coro);        \
//        int t=running_coro; running_coro=waiting_coro; waiting_coro=t; \
//        longjmp(envs[p2(running_coro)], a);      \
//    } \
//    else envs_state[running_coro]=0;

class coroutine;
class event_loop;

thread_local coroutine* running_coro = nullptr;
thread_local event_loop* curr_event_loop = nullptr;

extern "C" void swap64(coroutine *old_co, coroutine *new_co);
extern "C" void swap64v2(coroutine *old_co, coroutine *new_co);
extern "C" void swapback(coroutine *curr_co);
extern "C" void _co_end(coroutine *curr_co);
extern "C" void _save_ctx(coroutine *coro);    // 保存相关的CPU寄存器：rbx, rbp, r12-r15

struct coroutine_registers
{
    DWORD rdi;
    DWORD rbx;
    DWORD rbp;
    DWORD r12;
    DWORD r13;
    DWORD r14;
    DWORD r15;
    coroutine_registers(){
        rdi=0;
        rbx=0;rbp=0;
        r12=0;r13=0;r14=0;r15=0;
    }
};

void co_end();

void co_yield();

void coro_entry(std::function<void()>* f){
//    std::cout<<"this is a coro_entry\n";
    (*f)();
//    std::cout<<"exiting coro_entry...\n";
    co_end();
}


template<typename F, typename... Arg>
std::enable_if_t<std::is_same<void, typename std::result_of<F(Arg...)>::type>::value, std::function<void()>>
getWrapper(F&& _function, Arg&&... args) {
    using ret_type = typename std::result_of<F(Arg...)>::type;
    auto func = std::bind(std::forward<F>(_function), std::forward<Arg>(args)...);
    auto task = std::make_shared<std::packaged_task<ret_type()>>(func);
    std::future<ret_type> res = task->get_future();
    auto wrapper = [&](){};
    return true;
}
template<typename T>
std::enable_if_t<!std::is_same<void, T>::value, std::function<T()>> isVoid() {
    return false;
}

class coroutine{
    DWORD *sp;
    DWORD *stack_space;
    coroutine_registers regs;
public:
    coroutine *caller_coro;
    std::function<void()> wrapper;
    std::function<void()>* p;

    coroutine(){
        sp=0;
        stack_space=0;
        status=0;
        caller_coro=0;
    }

    template<typename F, typename... Arg>
    coroutine(F&& _function, Arg&&... args){

        getWrapper<F, Arg...>(std::forward<F>(_function), std::forward<Arg>(args)...);
        p = &wrapper;
        stack_space=new DWORD[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)+1]; // on macOS, plus 1 to avoid stack_not_16_byte_aligned_error
        sp=&stack_space[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)]; // on macOS, minus 2 to avoid stack_not_16_byte_aligned_error
        *(--sp)=(DWORD)co_end; // coroutine cleanup
//        std::cout<<"run_once addr:"<<sp<<std::endl;
        *(--sp)=(DWORD)coro_entry; // user's function to run_once (rop style!)
        regs.rdi = (DWORD)p;
//        std::cout<<"addr of regs.rdi:"<<&(regs.rdi)<<std::endl;
//        std::cout<<"addr of regs.rdx:"<<&(regs.rbx)<<std::endl;
        caller_coro = nullptr;
    }

    template<typename F, typename... Arg>
    std::enable_if_t<std::is_same<void, typename std::result_of<F(Arg...)>::type>::value, void> getWrapper(F&& _function, Arg&&... args) {
        using ret_type = typename std::result_of<F(Arg...)>::type;
        auto func = std::bind(std::forward<F>(_function), std::forward<Arg>(args)...);
        auto task = std::make_shared<std::packaged_task<ret_type()>>(func);
        std::future<ret_type> res = task->get_future();
        wrapper = [&](){
            status=1;
            func();
            status=0;
        };
        return;
    }

    template<typename F, typename... Arg>
    std::enable_if_t<!std::is_same<void, typename std::result_of<F(Arg...)>::type>::value, void> getWrapper(F&& _function, Arg&&... args) {
        using ret_type = typename std::result_of<F(Arg...)>::type;
        auto func = std::bind(std::forward<F>(_function), std::forward<Arg>(args)...);
        auto task = std::make_shared<std::packaged_task<ret_type()>>(func);
        std::future<ret_type> res = task->get_future();
        wrapper = [&](){
            status=1;
            ret_type ret = func();
            status=0;
            return ret;
        };
        return;
    }

    void reset(){
        /*
         * TODO: reset stack size
         */
        sp=&stack_space[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)-1]; // top of stack
        *(--sp)=(DWORD)co_end; // coroutine cleanup
        *(--sp)=(DWORD)coro_entry; // user's function to run_once (rop style!)
        regs.rdi = (DWORD)p;
        caller_coro = nullptr;
    }
    template<typename F, typename... Arg>
    void set(F&& _function, Arg&&... args){
        using ret_type = typename std::result_of<F(Arg...)>::type;
        auto func = std::bind(std::forward<F>(_function), std::forward<Arg>(args)...);
        wrapper = [&](){
            status=1;
            func();
            status=0;
        };
        p = &wrapper;
        sp=&stack_space[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)-1]; // top of stack
        *(--sp)=(DWORD)co_end; // coroutine cleanup
        *(--sp)=(DWORD)coro_entry; // user's function to run_once (rop style!)
        regs.rdi = (DWORD)p;
        caller_coro = nullptr;
    }
    void run() noexcept{
        if(status==1){
            std::cerr<<"Error running coroutine.\n";
            return;
        }
        status = 1;
        std::printf("starting to run_once...\n");
    }
    void stop() noexcept {
        status = 0;
        std::cout << "Coroutine stopped." << std::endl;
    }
    void next(){

    }
    friend class event_loop;
private:
//    std::function<void ()> wrapper;
    int status; // 0 for not started; 1 for running; 2 for suspended;
};

void co_end(){
    // TODO: reset
    _co_end(running_coro);
//    running_coro->reset();
}

//template<typename F>
class event_loop{
public:
    /*
     * TODO: local_co应该是
     */
    event_loop():running(false){
        local_coro = new coroutine();
    }

    explicit event_loop(std::queue<coroutine*> q, bool run_in_current_thread=true):running(false),t_loop(nullptr){
        local_coro = new coroutine();
        coros = q;
        run_until_complete(run_in_current_thread);
    }
    explicit event_loop(coroutine* coro, bool run_in_current_thread=true):running(false),t_loop(nullptr){
        local_coro = new coroutine();
        coros.push(coro);
        coro->caller_coro=local_coro;
        std::cout<<"addr of coro:"<<coro<<std::endl;
        std::cout<<"addr of coro's caller:"<<coro->caller_coro<<std::endl;
        run_until_complete(run_in_current_thread);
    }
    int run_until_complete(bool run_in_current_thread=true){
        if(!ALLOW_NESTED_ASYNC && curr_event_loop!=nullptr && curr_event_loop!=this){
            std::cerr<<"Nested asyncio is NOT ALLOWED.\n";
            return -1;
        }
        if(coros.empty()){
            return -1;
        }
        std::cout<<coros.size()<<std::endl;
        auto f_loop = [&](){
            curr_event_loop = this;
            running = true;
            do{
                running_coro=scheduler();
                swap64v2(local_coro, running_coro);
                if(running_coro->status!=0) coros.push(running_coro);
                running_coro=local_coro;
            } while(!coros.empty());
            running = false;
        };
        if(run_in_current_thread){
            f_loop();
        }
        else{
            t_loop = new std::thread(f_loop);
//            t_loop.detach();
        }
        return 0;
    }
    coroutine* scheduler(){
        auto nxt_co = coros.front();
        coros.pop();
        return nxt_co;
    }
    bool is_running(){ return running; }
    bool join(){ if(t_loop != nullptr){t_loop->join();return true;}return false;}
private:
    std::queue<coroutine*> coros;
    coroutine* local_coro;
    bool running;
    std::thread *t_loop;
};

void co_yield(){
    /*
     * TODO: 改成get_running_loop实现，减少thread local变量
     */
//    std::cout<<"running_coro:"<<running_coro<<std::endl;
    swapback(running_coro);
}

event_loop* get_running_loop(){
    /*
     * TODO: 该函数只能由协程或回调来调用，请包装成协程
     */
    if(!curr_event_loop->is_running()){
        throw std::runtime_error("Current event loop is not running!\n");
    }
    return curr_event_loop;
}

event_loop* get_event_loop(){
    /*
     * TODO: 该函数只能由协程或回调来调用，请包装成协程
     */
    return  curr_event_loop;
}

bool set_event_loop(event_loop* loop){
    /*
     * TODO
     */
    curr_event_loop = loop;
    return true;
}

/**
 * %rdi存的是old_co，也是 ”old_co->stack“的地址。(%rdi)就是old_co->stack
 * 例如old_co是0x123
 * old_co->stack是0x567
 * 那么old_co == &(old_co->stack) == 0x123
 * %rdi是0x123
 * (%rdi)是0x567
 * 令%rsp = (%rdi)
 * 则%rsp指向的地方是old_co->stack指向的地方
 *
 * "retq" = "popq %rip" 或者 "addq $8,%rsp; jmpq *-8(%rsp)"
 * "call" = "pushq 8(%rip)" 将下一条指令压栈
 */

__asm__(
".globl _swap64 \n\t"
".globl __save_ctx\n\t"
".globl _swap64v2\n\t"
".globl _swapback\n\t"
".globl __co_end\n\t"
"_swap64: \n\t"
"pushq %rdi \n\t"
"pushq %rbp \n\t"
"pushq %rbx \n\t"
"pushq %r12 \n\t"
"pushq %r13 \n\t"
"pushq %r14 \n\t"
"pushq %r15 \n\t"

"movq %rsp,(%rdi) \n\t" // old_co->stack = %rsp
"movq (%rsi),%rsp \n\t" // %rsp = new_co->stack

"popq %r15 \n\t"
"popq %r14 \n\t"
"popq %r13 \n\t"
"popq %r12 \n\t"
"popq %rbx \n\t"
"popq %rbp \n\t"
"popq %rdi \n\t"
//"addq $8,%rsp \n\t"
//"jmpq *-8(%rsp) \n\t"
"retq \n\t"

"__save_ctx:\n\t"
"movq %rbx,16(%rdi)\n\t"    // 往后指令依次给coro->regs.rbx rbp ... 赋值
"movq %rbp,24(%rdi)\n\t"
"movq %r12,32(%rdi)\n\t"
"movq %r13,40(%rdi)\n\t"
"movq %r14,48(%rdi)\n\t"
"movq %r15,56(%rdi)\n\t"
"retq\n\t"

"_swap64v2:\n\t"
"movq %rdi,16(%rdi) \n\t"
"movq %rbx,24(%rdi) \n\t"
"movq %rbp,32(%rdi) \n\t"
"movq %r12,40(%rdi) \n\t"
"movq %r13,48(%rdi) \n\t"
"movq %r14,56(%rdi) \n\t"
"movq %r15,64(%rdi) \n\t"

"movq %rsp,(%rdi) \n\t" // old_co->stack = %rsp
"movq (%rsi),%rsp \n\t" // %rsp = new_co->stack


"movq %rdi,72(%rsi) \n\t" // save caller coroutine
"movq 64(%rsi),%r15 \n\t"
"movq 56(%rsi),%r14 \n\t"
"movq 48(%rsi),%r13 \n\t"
"movq 40(%rsi),%r12 \n\t"
"movq 32(%rsi),%rbp \n\t"
"movq 24(%rsi),%rbx \n\t"
"movq 16(%rsi),%rdi \n\t"
//"popq %rdi \n\t"
//"addq $8,%rsp \n\t"
//"jmpq *-8(%rsp) \n\t"
"retq \n\t"

"_swapback:\n\t"
"movq 72(%rdi),%rsi\n\t"
"jmp _swap64v2\n\t"

"__co_end:\n\t"
"movq 72(%rdi),%rsi\n\t"
"movq (%rsi),%rsp \n\t" // %rsp = new_co->stack

"movq 64(%rsi),%r15 \n\t"
"movq 56(%rsi),%r14 \n\t"
"movq 48(%rsi),%r13 \n\t"
"movq 40(%rsi),%r12 \n\t"
"movq 32(%rsi),%rbp \n\t"
"movq 24(%rsi),%rbx \n\t"
"movq 16(%rsi),%rdi \n\t"
"retq \n\t"
);

#endif //JOBLIB_ASYNC_HPP
