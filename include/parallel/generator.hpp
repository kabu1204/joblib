//
// Created by yuchengye on 2021/11/7.
//

#ifndef JOBLIB_GENERATOR_HPP
#define JOBLIB_GENERATOR_HPP
#include "string"
#include "thread"
#include "chrono"
#include "memory"
#include "queue"
#include "future"
#include "csetjmp"
#include "easylogging/easylogging++.h"
#include <csetjmp>
#define GENERATOR_MAX_YIELDS_SIZE ~(1<<31)-1

template<typename F, typename... Args>
std::function<void ()> delayed(F&& function, Args&&...args){
//    std::function<void ()>* wrapped_func = std::make_shared<std::function<void ()> >(std::bind(std::forward<F>(function), std::forward<Args>(args)...));
    return std::bind(std::forward<F>(function), std::forward<Args>(args)...);
//    return *wrapped_func;
}

template<typename YieldType>
class generator: public std::enable_shared_from_this<generator<YieldType>>{
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
    explicit generator(F _function, int _sync=1):sync(_sync), status(0){
        sync = (_sync<=0)?GENERATOR_MAX_YIELDS_SIZE:sync;
        yield = yielder(this);
        auto wrapper = [&](F _function_){ _function(yield); stop(); };
        t = new std::thread(wrapper, _function);
//        t = new std::thread(_function, std::ref(yield));
        t->detach();
    }
    YieldType next();
    void stop();
    bool stopped();
    ~generator();

    class yielder{
    public:
        friend class generator<YieldType>;
        /**
         * @param _gen_ptr pointer of the Generator
         */
        yielder(generator<YieldType>* _gen_ptr = nullptr): gen_ptr(_gen_ptr) {
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
        void operator() (YieldType const& ret){
            std::unique_lock<std::mutex> lk(gen_ptr->Mtx);
            if(gen_ptr->status<0){
                std::terminate();
            }
            gen_ptr->cv.wait(lk, [&]{return gen_ptr->yields.size()<gen_ptr->sync;});
            gen_ptr->yields.push(ret);
            lk.unlock();
            gen_ptr->cv.notify_one();
        }
    private:
        generator<YieldType>* gen_ptr;
    };

private:
    yielder yield;
    int status;
    int sync;
    std::queue<YieldType> yields;
    std::mutex Mtx;
    std::condition_variable cv;
    std::thread* t;
};

/**
 * When next() is called, it will pop a ret from the queue yields.
 * @tparam T
 * @return
 * TODO: 可以考虑不用队列
 */
template<typename T>
T generator<T>::next() {
    std::unique_lock<std::mutex> lk(Mtx);
    if(status<0){
        LOG(DEBUG)<<"StopIterationException";
        throw std::runtime_error("StopIterationException: This generator has already stopped.");
    }
    cv.wait(lk, [&]{return !yields.empty();});
    auto ret = yields.front();
    yields.pop();
    lk.unlock();
    cv.notify_one();
    return ret;
}

template<typename T>
void generator<T>::stop() {
    std::unique_lock<std::mutex> lk(Mtx);
    if (status < 0) {
        LOG(DEBUG) << "StopIterationException";
        throw std::runtime_error("StopIterationException: This generator has already stopped.");
    }
    status = -1;
    lk.unlock();
}

template<typename T>
generator<T>::~generator() {
    stop();
}

template<typename YieldType>
bool generator<YieldType>::stopped() {
    return status<0;
}


#endif //JOBLIB_GENERATOR_HPP
