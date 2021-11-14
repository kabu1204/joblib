#include "main.h"

void resize_image(){}
void read_image(){}
void save_image(){}

coroutine *main_co;
coroutine *sub_co;

void test_for_coro(){
    main_co=new coroutine();
    sub_co=new coroutine([=](){
        int i;
        std::cout<<"\tin co1 first\n";
        swap64(sub_co, main_co);
        std::cout<<"\tin co1 twice\n";
        swap64(sub_co, main_co);
        return;
    });
    std::cout<<"this is main coro!\n"<<std::endl;
    swap64(main_co, sub_co);
    std::cout<<"this is main coro!\n"<<std::endl;
    swap64(main_co, sub_co);
    std::cout<<"all end\n"<<std::endl;
}

void test_for_async(){
    std::vector<std::function<void()> > vec;
    for(auto i:Range(2)){
        auto func = [=](){
            std::printf("child coro%d begin\n", i);
            running_coro = i+1; waiting_coro = 0;
            yield(999);
            std::printf("middle%d %d %d %d\n", i, envs_state[0], envs_state[1], envs_state[2]);
            yield(998);
            std::printf("child coro%d begin\n", i);
            coro_ret();
        };
        vec.push_back(func);
    }
    event_loop<void()> loop(vec);
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
//    using ret_t = decltype(getRetValue(tf));
    test_for_coro();
//    test_for_async();
//    test_for_parallel();
//    test_for_generator();
//    test_for_threadpool();
//    b();
}
