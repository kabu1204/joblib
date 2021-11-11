//
// Created by yuchengye on 2021/11/9.
//

#ifndef JOBLIB_THREADPOOL_HPP
#define JOBLIB_THREADPOOL_HPP
#include "range.h"
#include "vector"
#include "queue"
#include "thread"
#include "future"
#include "mutex"
#include "memory"
#include "type_traits"
#include "functional"
#include "condition_variable"
#include "easylogging/easylogging++.h"
#define THREADPOOL_MAX_TASKS_SIZE ~(1<<31)-1

class threadpool{
public:
    /**
     * @brief 创建一个线程池
     * @param _n_thread 创建的worker线程数量。默认为1。
     * @param _n_tasks 处于等待队列中的任务的最大数量。小于0时代表无限制。默认为-1。
     */
    threadpool(uint32_t _n_thread=1, uint32_t _n_tasks=-1):n_thread(_n_thread), status(0){
        n_tasks = (_n_tasks<=0)?THREADPOOL_MAX_TASKS_SIZE:_n_tasks;
        for(auto i:Range(n_thread)){
            workers.emplace_back([&]{
                while(true){
                    std::unique_lock<std::mutex> lk(Mtx);
                    if(status<0) break;
//                    while(tasks.empty()) cv.wait(lk);
                    cv.wait(lk, [&]{return !tasks.empty();});
                    auto task = tasks.front();
                    tasks.pop();
                    cv.notify_all();
                    lk.unlock();
                    task();
                }
            });
        }
    }
    /**
     * @brief 添加任务，如果线程池已停止，则会抛出失败的错误
     * @tparam F std::function<void ()>
     * @tparam Args 参数类型
     * @param function 函数
     * @param args 参入函数的参数
     * @return
     */
    template<typename F, typename... Args>
    auto add_jobs(F&& function, Args&&...args){
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(function), std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        std::unique_lock<std::mutex> lk(Mtx);
        if(status<0){
            LOG(DEBUG)<<"threadpool stopped";
            throw std::runtime_error("threadpool stopped");
        }
//        while(tasks.size()>=n_tasks) cv.wait(lk);
        LOG(INFO)<<"tasks.size() "<<tasks.size();
        cv.wait(lk, [&]{return tasks.size()<n_tasks;});
        tasks.emplace([task](){ (*task)(); });
        lk.unlock();
        cv.notify_one();
        return res;
    }

//    template<typename F>
//    auto add_jobs(F&& wrapped_func){
//        using return_type = typename std::result_of<F()>::type;
//        auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(wrapped_func));
//        std::future<return_type> res = task->get_future();
//        std::unique_lock<std::mutex> lk(Mtx);
//        if(status<0){
//            LOG(DEBUG)<<"threadpool stopped";
//            throw std::runtime_error("threadpool stopped");
//        }
//        LOG(INFO)<<"tasks.size() "<<tasks.size();
//        cv.wait(lk, [&]{return tasks.size()<n_tasks;});
//        tasks.emplace([task](){ (*task)(); });
//        lk.unlock();
//        cv.notify_one();
//        return res;
//    }
    void stop(){
        std::unique_lock<std::mutex> lk(Mtx);
        status=-1;
        lk.unlock();
    }
private:
    uint32_t n_thread;
    uint32_t n_tasks;
    uint8_t status;
    std::mutex Mtx;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    std::queue<std::function<void ()> > tasks;
};

#endif //JOBLIB_THREADPOOL_HPP
