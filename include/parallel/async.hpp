//
// Created by yuchengye on 2021/11/11.
//

#ifndef JOBLIB_ASYNC_HPP
#define JOBLIB_ASYNC_HPP
#define DEFAULT_CORO_STACK_SIZE 1024*1024
#define MAX_COROUTINES_PER_LOOP 1024
#define MAIN_CORO 0
#include "thread"
#include "string"
#include "chrono"
#include "cstdio"
#include "csetjmp"
#include "iostream"

int p(int x){std::printf("saving env %d\n",x); return x;};
int p2(int x){std::printf("jumping env %d\n",x); return x;};

#define yield(a) setjmp(envs[p(running_coro)]); \
    if(envs_state[running_coro]==0) {           \
        envs_state[running_coro]=1;             \
        jmpable_coros.push(running_coro);        \
        int t=running_coro; running_coro=waiting_coro; waiting_coro=t; \
        longjmp(envs[p2(running_coro)], a);      \
    } \
    else envs_state[running_coro]=0;

typedef int64_t DWORD;
typedef int32_t LWORD;
typedef int16_t DBYTE;
typedef int8_t BYTE;

class coroutine;

thread_local int running_coro = MAIN_CORO;
thread_local int waiting_coro = MAIN_CORO + 1;
thread_local jmp_buf envs[MAX_COROUTINES_PER_LOOP];
thread_local int envs_state[MAX_COROUTINES_PER_LOOP]={0};
thread_local std::queue<int> jmpable_coros;
thread_local coroutine* main_coro;

template<typename F>class event_loop;

void coro_end(){
    return;
}

void tf(std::function<void()>* f){
    std::cout<<"this is a test func\n";
    (*f)();
}

class coroutine{
    DWORD *sp;
    DWORD *stack_space;
    std::function<void()> wrapper;
    std::function<void()>* p;
public:
    coroutine(){
        sp=0;
        stack_space=0;
    }

    template<typename F, typename... Arg>
    coroutine(F&& _function, Arg&&... args){
        auto func = std::bind(std::forward<F>(_function), std::forward<Arg>(args)...);
        wrapper = [&](){
            func();
            stop();
        };
        p = &wrapper;
        stack_space=new DWORD[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)];
        sp=&stack_space[DEFAULT_CORO_STACK_SIZE/sizeof(DWORD)-1]; // top of stack
        *(--sp)=(DWORD)coro_end; // coroutine cleanup
        std::cout<<"run addr:"<<sp<<std::endl;
        *(--sp)=(DWORD)tf; // user's function to run (rop style!)
        *(--sp)=(DWORD)p;
        std::cout<<"addr of wrapper:"<<p<<std::endl;
        for (int saved=0;saved<6;saved++)
            *(--sp)=0xdeadbeef; // initial values for saved registers
    }
    void run(){
        std::printf("run end\n");
    }
    void stop(){
        std::cout<<"Coroutine end.\n";
    }
    void next(){

    }

private:
//    std::function<void ()> wrapper;
//    int status;
};

template<typename F>
class event_loop{
public:
    explicit event_loop(std::vector<std::function<F>> _coros){
        std::memset(envs_state, -1, sizeof(envs_state));
        envs_state[MAIN_CORO] = 0;
        for(auto i:Range(_coros.size())){
            jmpable_coros.push(i+1);
        }
        int res = setjmp(envs[MAIN_CORO]);
        if(!jmpable_coros.empty()) waiting_coro = scheduler();
        else return;    // complete
        std::printf("current waiting coro:%d[%d %d %d]\n", waiting_coro, envs_state[0], envs_state[1], envs_state[2]);
        if(envs_state[waiting_coro]==-1){
            envs_state[waiting_coro]=0;
            std::printf("starting to run coro:%d\n", waiting_coro);
            _coros[waiting_coro-1]();
        }
        int t = waiting_coro; waiting_coro = running_coro; running_coro = t;
        longjmp(envs[running_coro],1);
    }
//    event_loop(coroutine<F> coro){
//        std::memset(envs_state, 0, sizeof(envs_state));
//        std::printf("[main coro]ok\n");
//        running_coro = 0; waiting_coro=1;
//        int res = setjmp(envs[running_coro]);
//        if(res==0)
//        {coro.run();}
//        std::printf("back to main coro %d %d\n", envs_state[0], envs_state[1]);
//        yield(10);
//        std::printf("main coro end\n");
//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//        yield(12);
//    }
    int scheduler(){
        waiting_coro = jmpable_coros.front();
        jmpable_coros.pop();
        return waiting_coro;
    }
private:
};

inline void coro_ret(){
    longjmp(envs[MAIN_CORO], 1);
}

extern "C" void swap64(coroutine *old_co, coroutine *new_co);
extern "C" void _save_ctx(coroutine *coro);    // 保存相关的CPU寄存器：rbx, rbp, r12-r15

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
"retq"
);

#endif //JOBLIB_ASYNC_HPP
