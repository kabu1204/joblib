//
// Created by yuchengye on 2021/11/17.
//

#include "parallel/async_stackless.h"
using namespace stackless;

struct stackless::co{
    stackless::co *co_v_parent;
    std::set<stackless::co*> co_v_childs;    // TODO 考虑链表实现
    bool isRootCo;
    uint8_t co_v_status;
    uint32_t co_v_state;
    uint8_t co_v_state_cnt;
public:
    void* _res_;
    co(stackless::co * _parent=nullptr, bool _isRootCo=false):co_v_status(SUSPENDED), co_v_state(0), co_v_state_cnt(0),
    isRootCo(_isRootCo){
        if(!isRootCo) co_v_parent=_parent;
    }
    void mount_on(stackless::co* _parent){
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
    void unmount_from(){
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
    void mount(stackless::co *_child){
        // TODO: 注意判重
        co_v_childs.insert(_child);
    }
    void unmount(stackless::co *_child){
        co_v_childs.erase(_child);
    }

    template<class T>
    void await(stackless::co *new_co){
        new_co->mount_on(this);
        // call scheduler
    }
    template<class T, class... Arg>
    void run(Arg... args){
        static_cast<T*>(this)->_co_func_(args...);
    }

    void clean(){
        if(!co_v_childs.empty()){
            std::cerr<<"To reset an internal coroutine node is NOT ALLOWED.\n"<<std::endl;
            return;
        }
        co_v_state=0;
        if(!isRootCo) unmount_from();
    }
};
// TODO: 返回值的存储，指针类型的转换，调用派生类函数
struct co1:public stackless::co{
    //1: Declare local variables you need
    int a,b,i;

    //2: Declare function and implement it like this
    CO_DEF(int, int c)  // function ProtoType
        a=c;            // DO NOT USE parameters directly, it will not be saved; save it in a local variable;
        for(i=0;i<=10;++i) {
            CO_YIELD(i);       // yield wherever you need to
        }
        CO_RET(11);     // return value
    CO_DEF_END;         // be sure to CO_DEF_END at end

};

/**
 * @brief 同一async_task的coroutine保证具有正确的关系
 */
class stackless::async_task{
private:
    char *task_name;
    uint8_t status;
    stackless::co* root_co;
    stackless::co* running_co;
public:
    async_task(stackless::co *co, const char *name=nullptr){
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
    stackless::co* get_running_co(){
        if(status==RUNNING){
            return running_co;
        }
        std::cerr<<"There is no running coroutine."<<std::endl;
        return nullptr;
    }

    /**
     * 从running_co切换至dst_co (调用)。该函数只应该由event_loop调用
     * @param new_co
     * @return
     */
private:
    friend class event_loop;
    bool switch_to(stackless::co* dst_co){
        // TODO:多线程调度请加锁
        dst_co->co_v_state=RUNNING;
        running_co->co_v_state=SUSPENDED;
        running_co=dst_co;
        return true;
    }
    void run(){
        if(running_co){
//            running_co();
        }
    }
};

stackless::event_loop* stackless::get_running_loop() {
    if(running_loop && running_loop->status==RUNNING){
        return running_loop;
    }
    std::cerr<<"There is no running event loop in current thread."<<std::endl;
    return nullptr;
}

template<typename T, typename... Arg>
void stackless::_co_await(stackless::co* old_co, stackless::co* new_co, Arg... args) {
    // TODO: status setting
    T* _new_co = static_cast<T*>(old_co);
    _new_co->_co_func_(std::forward<Arg>(args)...);
    stackless::event_loop* loop=stackless::get_running_loop();
    stackless::async_task* task=loop->get_running_task();
    stackless::co* src_co=task->get_running_co();

    src_co->co_v_status=SUSPENDED;
    new_co->mount_on(src_co);
    // TODO:此时task已不是running状态，考虑是否有必要设置status和running_co
    // goto scheduler
}

template<typename T>
void stackless::_co_await(stackless::co* old_co, stackless::co* new_co) {
    // TODO: status setting
//    T* _new_co = static_cast<T*>(old_co);
    T* _new_co = (T*)old_co;
    _new_co->_co_func_();
    stackless::event_loop* loop=stackless::get_running_loop();
    stackless::async_task* task=loop->get_running_task();
    stackless::co* src_co=task->get_running_co();

    src_co->co_v_status=SUSPENDED;
    new_co->mount_on(src_co);
    // TODO:此时task已不是running状态，考虑是否有必要设置status和running_co
    // goto scheduler
}

void test(){
    co1* co_ptr = new co1();
    stackless::_co_await<co1>(co_ptr, co_ptr, 1);
}
