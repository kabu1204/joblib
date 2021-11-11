//
// Created by yuchengye on 2021/11/11.
//

#ifndef JOBLIB_ASYNC_HPP
#define JOBLIB_ASYNC_HPP
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

thread_local int running_coro = MAIN_CORO;
thread_local int waiting_coro = MAIN_CORO + 1;
thread_local jmp_buf envs[MAX_COROUTINES_PER_LOOP];
thread_local int envs_state[MAX_COROUTINES_PER_LOOP]={0};
thread_local std::queue<int> jmpable_coros;

template<typename F>class event_loop;

template<typename F>
class coroutine{
public:
//    friend class event_loop<F>;
    coroutine(std::function<F>&& _function){
        wrapper = [&](){ _function(); stop(); };
    }
    void run(){
        wrapper();
        std::printf("run end\n");
    }
    void stop(){
        status = -1;
    }
    void next(){

    }

private:
    std::function<void ()> wrapper;
    int status;
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

#endif //JOBLIB_ASYNC_HPP
