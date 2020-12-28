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
    pthread_t *threadpool;
    std::list<T*> workqueue;
    locker m_locker;
    sem m_sem;
    bool is_stop;

private:
    static void *worker(void* arg);
    void run();

public:
    cstyle_threadpool(int number_thread, int number_request);
    ~cstyle_threadpool();
    bool append(T* request);
};

template <typename T>
cstyle_threadpool<T>::cstyle_threadpool(int number_thread, int number_request) : max_thread(number_thread), max_request(number_request) {
    assert(number_thread > 0 && number_request > 0);
    threadpool = new threadpool[max_thread];
    if (!threadpool) {
        throw std::exception();
    }
    for (int i = 0; i < max_thread; ++i) {
        // 创建线程
        if (pthread_create(threadpool+i, NULL, worker, this) != 0) {
            delete [] threadpool;
            throw std::exception();
        }
        // 主线程与子线程分离,子线程结束后,资源自动回收
        if (pthread_detach(threadpool[i])) {
            delete[] threadpool;
            throw std::exception();
        }
    }
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