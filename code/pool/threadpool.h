/*
 * @Author       : mark
 * @Date         : 2020-06-15
 * @copyleft Apache 2.0
 */ 

#ifndef THREADPOOL_H
#define THREADPOOL_H

//#include <mutex>
//#include <condition_variable>
//#include <queue>
//#include <thread>
//#include <functional>
//
//class ThreadPool {
//public:
//    explicit ThreadPool(size_t threadCount = 8): pool_(std::make_shared<Pool>()) {
//            assert(threadCount > 0);
//            for(size_t i = 0; i < threadCount; i++) {
//                std::thread([pool = pool_] {
//                    std::unique_lock<std::mutex> locker(pool->mtx);
//                    while(true) {
//                        if(!pool->tasks.empty()) {
//                            auto task = std::move(pool->tasks.front());
//                            pool->tasks.pop();
//                            locker.unlock();
//                            task();
//                            locker.lock();
//                        }
//                        else if(pool->isClosed) break;
//                        else pool->cond.wait(locker);
//                    }
//                }).detach();
//            }
//    }
//
//    ThreadPool() = default;
//
//    ThreadPool(ThreadPool&&) = default;
//
//    ~ThreadPool() {
//        if(static_cast<bool>(pool_)) {
//            {
//                std::lock_guard<std::mutex> locker(pool_->mtx);
//                pool_->isClosed = true;
//            }
//            pool_->cond.notify_all();
//        }
//    }
//
//    template<class F>
//    void AddTask(F&& task) {
//        {
//            std::lock_guard<std::mutex> locker(pool_->mtx);
//            pool_->tasks.emplace(std::forward<F>(task));
//        }
//        pool_->cond.notify_one();
//    }
//
//private:
//    struct Pool {
//        std::mutex mtx;
//        std::condition_variable cond;
//        bool isClosed;
//        std::queue<std::function<void()>> tasks;
//    };
//    std::shared_ptr<Pool> pool_;
//};
#include <condition_variable>
#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class CAS{
public:
    CAS() : flag(false) {}

    void lock(){
        bool expect = false;
        while(!flag.compare_exchange_weak(expect, true)){
            expect = false;
        }
    }
    void unlock(){
        flag.store(false);
    }
private:
    CAS(const CAS&) = default;
    CAS& operator=(const CAS&) = delete;
    std::atomic<bool> flag;
};

class Thread{
public:
    Thread(){
        fg = 0 ;
    }
    Thread(std::function<void()> func) : func_(func),fg(1){

    }

    void start() {
        func_();
    }

    int fg = 0;

private:
    std::function<void()> func_;
};

class ThreadPool
{
public:
    explicit ThreadPool(size_t Thread_sum = 8)  {
        assert(Thread_sum>0);
        pool_ = new std::queue<Thread>;
        for(size_t i=0;i<Thread_sum;i++){
            std::thread([this]{
                while(true){
                    Thread task;
                    mutex.lock();
                    if(!pool_->empty()){
                        task = std::move(pool_->front());
                        pool_->pop();
                    }
                    mutex.unlock();

                    if (task.fg == 1 ){
                        task.start();
                    }
                }
            }).detach();
        }
    }
    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool(){

    }

    template<class F>
    void AddTask(F&& task){
        mutex.lock();
        pool_->push(Thread(task));
        mutex.unlock();
    }

private:
    std::queue<Thread> *pool_;
    CAS mutex;
};


#endif //THREADPOOL_H