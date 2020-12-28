#ifndef CSTYLE_THREAD_POOL_H
#define CSTYLE_THREAD_POOL_H

#include <pthread.h>
#include <exception>
#include <list>
#include <assert.h>
#include <../lock/locker.h>

template <typename T>
class cstyle_threadpool
{
private:
    int max_thread;
    int max_request;
    pthread_t threadpool = pthread_t[max_thread];
    std::list<T*> workqueue;
    locker m_locker;
    sem m_sem;
    bool is_stop;

private:
    static void* worker(void* arg);
    void run();

public:
    cstyle_threadpool(int number_thread = 8, int number_request = 1000);
    ~cstyle_threadpool();
    bool append(T* request);
};

template <typename T>
cstyle_threadpool<T>::cstyle_threadpool(int number_thread = 8, int number_request = 1000) {
    assert(number_thread > 0 && number_request > 0);

}

template <typename T>
cstyle_threadpool<T>::~cstyle_threadpool() {
    delete [] threadpool;
    is_stop = true;
}

template <typename T>
bool cstyle_threadpool<T>::append(T* request) {
    if (is_stop) {
        return false;
    }
    m_locker.lock();
    if (workqueue.size() > max_request) {
        m_locker.unlock();
        return false;
    }
    workqueue.push_back(request);
    m_locker.unlock();
    m_sem.post();
    return true;
}

template <typename T>
void* cstyle_threadpool<T>::worker(void* arg) {
    cstyle_threadpool* pool = (cstyle_threadpool*)arg;
    pool->run;
    return pool;
}

template <typename T>
void cstyle_threadpool<T>::run() {
    while (!is_stop) {
        m_sem.wait();               // 信号量-1
        m_locker.lock();            // 互斥量加锁
        if (workqueue.empty()) {    // 请求队列为空则解锁并继续循环
            m_locker.unlock();
            continue;
        }
        T* request = workqueue.front();
        workqueue.pop_front();
        m_locker.unlock();          // 对临界区操作后解锁
        if (request == nullptr) {
            continue;
        }
        else {
            request.precess();     // 执行请求
        }
    }
}

#endif