#include "main.h"

void resize_image(){}
void read_image(){}
void save_image(){}

coroutine *main_co;
coroutine *sub_co;

CO_STKLESS(co1)
    //1: Declare local variables you need
    int a,b,i;
    //2: Declare function and implement it like this
CO_DEF(int, int c)  // function ProtoType
    a=c;
    std::cout<<a<<std::endl;
    CO_RET(c);
DEF_END;         // be sure to DEF_END at end

CO_STKLESS(co2)
    //1: Declare local variables you need
    int a,b,i;
    //2: Declare function and implement it like this
    CO_DEF(int)  // function ProtoType
        a= CO_AWAIT(co1,10);
        std::cout<<"get from co1:"<<a<<std::endl;
        CO_RET(a);
DEF_END;         // be sure to DEF_END at end

GEN_STKLESS(gen1,int,int)
int i;
int b;
    GEN_DEF(int c)
        for(i=0;i<10;++i){
            CO_YIELD(i,b);
//            std::cout<<b<<std::endl;
        }
DEF_END;

void test_for_gen_stkless(){
    gen1 g;
    g(1);
    for(auto i:Range(20)){
        std::cout<<g.next()<<std::endl;
    }
}

void test_for_co_stkless(){
    co2 *p=new co2();
    p->isRootCo=true;

    stackless::async_task* task = new stackless::async_task(p);

    stackless::event_loop_s* loop = new stackless::event_loop_s(task);
    running_loop = loop;
    running_loop->status = RUNNING;
    running_loop->running_task_idx=0;

    task->run();
}

void test_for_eventloop(){
    std::cin.tie(nullptr);
//    sub_co = new coroutine([=](){
//        int i;
//        std::cout<<"\tin co1 first\n";
//        std::cout<<"saved caller's coro:"<<running_coro->caller_coro<<std::endl;
//        co_yield();
//        std::cout<<"\tin co1 twice\n";
//        co_yield();
//        return;
//    });
    clock_t start = std::clock();
    std::queue<coroutine*> vec;
    coroutine *new_co;
    int num_co=10240;
    for(int i=0;i<num_co;++i){
        new_co = new coroutine([=](){
            int a[32000];
//            std::cout<<"\tin co"<<i<<" first"<<std::endl;
//            std::cout<<"saved caller's coro:"<<running_coro->caller_coro<<std::endl;
            co_yield();
//            std::cout<<"\tin co"<<i<<" twice"<<std::endl;
            co_yield();
            return 1;
        });
        vec.push(new_co);
    }
    event_loop loop(vec, true);
    clock_t end = std::clock();
    std::cout<<1000*1000*double(end-start)/CLOCKS_PER_SEC/num_co<<"us/co"<<std::endl;
//    loop.join();
//    event_loop_s l(sub_co);
}

void test_for_coro(){
    main_co=new coroutine();
    sub_co=new coroutine([=](){
        int i;
        std::cout<<"\tin co1 first\n";
        swap64v2(sub_co, main_co);
        std::cout<<"\tin co1 twice\n";
        swap64v2(sub_co, main_co);
        return;
    });
    std::cout<<"addr of sub_co:"<<sub_co<<std::endl;
    std::cout<<"this is main coro!\n"<<std::endl;
    swap64v2(main_co, sub_co);
    std::cout<<"this is main coro!\n"<<std::endl;
    swap64v2(main_co, sub_co);
    std::cout<<"all end\n"<<std::endl;
}

void test_for_async(){
//    std::vector<std::function<void()> > vec;
//    for(auto i:Range(2)){
//        auto func = [=](){
//            std::printf("child coro%d begin\n", i);
//            running_coro = i+1; waiting_coro = 0;
//            yield(999);
//            std::printf("middle%d %d %d %d\n", i, envs_state[0], envs_state[1], envs_state[2]);
//            yield(998);
//            std::printf("child coro%d begin\n", i);
//            coro_ret();
//        };
//        vec.push_back(func);
//    }
//    event_loop_s<void()> loop(vec);
}

void test_for_parallel(){
#undef yield(a)
    Parallel(2)(generator<std::function<void ()>>([&](auto yield){
        yield(delayed([=](){ LOG(INFO)<<"test for parallel";}));
        for(auto i:Range(10000)){
            if(i%2==0)
                yield(delayed([=](){std::cout<<i<<std::endl;}));
        }
        yield(delayed([](){
            read_image();
            resize_image();
            save_image();
        }));
    }));
}

void test_for_generator(){
    //    Parallel(2)(Range(10));
    LOG(INFO)<<apply([=](int x, int y)->int{return x+y;});
    generator<std::function<void ()>> g([&](auto yield){
        for(auto i:Range(10000)){
            yield(delayed([]() { std::cout<<"ok\n";}));
        }
    }, 1);
    LOG(INFO)<<"OK";
    for(auto i:Range(10000)){
        g.next()();
    }
//    g.next();
}

void test_for_threadpool(){
    threadpool pool(10, 1);
    LOG(INFO)<<"start";
    for(auto i:Range(10))
        for(auto j:Range(10)) {
            auto res = pool.add_jobs([]() { std::cout<<"ok\n";});
        }
    LOG(INFO)<<"mid";
//    for(auto i:Range(10))
//        for(auto j:Range(10)) {
//            std::this_thread::sleep_for(std::chrono::milliseconds(500));
//        }
    int i = ~(1<<31);
    LOG(INFO)<<~(1<<31);
}

jmp_buf abuf;
jmp_buf bbuf;

void a(){
    int res = setjmp(abuf);
    if(res==0){
        std::printf("first setjmp a\n");
        longjmp(bbuf,1);
    }
    else if(res==1){
        std::printf("abuf res 1\n");
        longjmp(bbuf,2);
    }
    else if(res==2){
        std::printf("abuf res 2\n");
        longjmp(bbuf,3);
    }
    std::printf("end of a()\n");
}

void b(){
    int res = setjmp(bbuf);
    if(res==0)
        a();
    else if(res==1)
        longjmp(abuf, 1);
    else if(res==2)
        longjmp(abuf, 2);
    else
        longjmp(abuf, 3);
}

template <class R, class... Args>
R getRetValue(R(*)(Args...));

int main(){
//    auto a = [](int c){return 1;};
//    using ret_t = decltype(getRetValue(coro_entry));
//test_for_co_stkless();
test_for_gen_stkless();
//    test_for_eventloop();
//    test_for_coro();
//    test_for_async();
//    test_for_parallel();
//    test_for_generator();
//    test_for_threadpool();
//    b();
}
