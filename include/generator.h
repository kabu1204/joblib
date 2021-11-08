//
// Created by yuchengye on 2021/11/7.
//

#ifndef JOBLIB_GENERATOR_H
#define JOBLIB_GENERATOR_H
#include "string"
#include "thread"
#include "chrono"
#include "memory"
#include "queue"
#include "future"
#include "easylogging/easylogging++.h"

template<typename T>
class generator: public std::enable_shared_from_this<generator<T>>{
    /**
     * TODO: 性能优化
     */
public:
    /**
     * @brief the constructor of generator
     * @tparam F std::function
     * @param _function a function with prototype: Any func(generator<T>::yielder yield)
     * @param _sync the max_size of buffered yield outputs
     */
    template<typename F>
    explicit generator(F _function, int _sync=1):sync(_sync){
        sync = (_sync<=0)?999999:sync;
        yield = yielder(this);
        auto wrapper = [&](F _function_){ _function(yield); status=-1; };
        t = new std::thread(wrapper, _function);
//        t = new std::thread(_function, std::ref(yield));
        t->detach();
    }
    T next();
    void stop();

    class yielder{
    public:
        friend class generator<T>;
        /**
         * @param _gen_ptr pointer of the Generator
         */
        yielder(generator<T>* _gen_ptr = nullptr):gen_ptr(_gen_ptr) {
            if(gen_ptr == nullptr){
                return;
            }
            LOG(INFO)<<gen_ptr->status;
        }

        /**
         * @brief When yield(ret) is called, it will push the ret into the queue yields.
         * Then the generator function will be suspended, wating to be waked up by next() of the generator.
         * @param ret the object to yield
         * @return ret
         */
        void operator() (T const& ret){
            if(gen_ptr->status<0){
                std::terminate();
            }
            std::unique_lock<std::mutex> lk(gen_ptr->Mtx);
            gen_ptr->cv.wait(lk, [&]{return gen_ptr->yields.size()<gen_ptr->sync;});
            gen_ptr->yields.push(ret);
            lk.unlock();
            gen_ptr->cv.notify_one();
        }
        generator<T>* gen_ptr;
    };

private:
    yielder yield;
    int status;
    int sync;
    std::queue<T> yields;
    std::mutex Mtx;
    std::condition_variable cv;
    std::thread* t;

    template<typename F>
    void wrapper(F _function){
        _function(yield);

    }
};

/**
 * When next() is called, it will pop a ret from the queue yields.
 * @tparam T
 * @return
 * TODO: 可以考虑不用队列
 */
template<typename T>
T generator<T>::next() {
    if(status<0){
        LOG(DEBUG)<<"StopIterationException";
        throw std::runtime_error("StopIterationException: This generator has already stopped.");
    }
    std::unique_lock<std::mutex> lk(Mtx);
    cv.wait(lk, [&]{return !yields.empty();});
    auto ret = yields.front();
    yields.pop();
    LOG(INFO)<<"poped:"<<ret<<"size of yields:"<<yields.size();
    lk.unlock();
    cv.notify_one();
    return ret;
}

template<typename T>
void generator<T>::stop() {
    if(status<0){
        LOG(DEBUG)<<"StopIterationException";
        throw std::runtime_error("StopIterationException: This generator has already stopped.");
    }
    status = -1;
}

#endif //JOBLIB_GENERATOR_H
