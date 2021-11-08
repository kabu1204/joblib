#include "main.h"

void test_for_generator(){
    //    Parallel(2)(Range(10));
    LOG(INFO)<<apply([=](int x, int y)->int{return x+y;});
    generator<int> g([](generator<int>::yielder yield){
        for(auto i:Range(10000)){
            yield(i);
        }
    }, 1);
    LOG(INFO)<<"OK";
    for(auto i:Range(10000)){
        g.next();
    }
    g.next();
}

int main(){
    test_for_generator();
}
