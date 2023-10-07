#pragma once
#include <iostream>
#include <string>
#include <queue>
#include <pthread.h>

namespace MyUtility
{
    const int NUM = 5;  //总线程数

    template <class T>
    class ThreadPool
    {
    private:
        int _num;                           //线程数量
        std::queue<T> _task_queue;          //任务队列，所有线程处理的资源
        pthread_mutex_t _mtx;               //互斥锁 线程间竞争任务
        pthread_cond_t  _existence;         //条件变量 在任务队列为空时 线程在 existence条件变量下等待
        static ThreadPool<T>* _pThreadPool; //线程池单例

    private:
        //单例模式：不让类定义对象 拷贝构造 赋值
        ThreadPool(int num = NUM)
            :_num(num)
        {
            pthread_mutex_init(&_mtx, nullptr);
            pthread_cond_init(&_existence, nullptr);
        }

        ThreadPool(const ThreadPool<T>& tp) = delete;

        ThreadPool<T>& operator=(const ThreadPool<T>& tp) = delete; 

        void InitThreadPool(void)
        {
            pthread_t tid;
            
            for(size_t i = 0; i < _num; i++)
            //创建 _num 个线程，所有线程去任务队列中拿数据
            {
                pthread_create(&tid, nullptr, Routine, (void*)this);
            }
        }

    public:
        static ThreadPool<T>* GetThreadPool(void)
        {
            static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            if(_pThreadPool == nullptr)
            {
                pthread_mutex_lock(&lock);
                if(_pThreadPool == nullptr)
                {
                    _pThreadPool = new ThreadPool<T>();
                    _pThreadPool->InitThreadPool();
                }
                pthread_mutex_unlock(&lock);
            }

            return _pThreadPool;
        }

    public:
        void Lock(void)
        {
            pthread_mutex_lock(&_mtx);
        }

        void Unlock(void)
        {
            pthread_mutex_unlock(&_mtx);
        }

        void Wait(void)
        {
            pthread_cond_wait(&_existence, &_mtx);
        }

        void WakeUp(void)
        {
            pthread_cond_signal(&_existence);
        }

        void WakeUpAll(void)
        {
            pthread_cond_broadcast(&_existence);
        }

        bool IsEmpty(void)
        {
            return _task_queue.size() == 0 ? true : false;
        }


   public:
        void PushTask(const T& in)
        {
            Lock();
            _task_queue.push(in);
            Unlock();
            WakeUp();
        }

        void PopTask(T* out)
        {
            //这里没上锁的原因是：
            //在Routine中线程拿任务的过程是上锁的（串行的）

            *out = _task_queue.front();
            _task_queue.pop();
        }

        static void* Routine(void* args)
        {
            pthread_detach(pthread_self());
            ThreadPool<T>* pThis = (ThreadPool<T>*)args; 

            //不停的处理任务
            while(true)
            {
                pThis->Lock();

                //任务队列为空时，让线程挂起
                while(pThis->IsEmpty())
                {
                    pThis->Wait();
                }

                //存在任务，线程取走任务，并执行
                T t;
                pThis->PopTask(&t);
                pThis->Unlock();

                t.Run();
            }
        }

        ~ThreadPool()
        {
            pthread_mutex_destroy(&_mtx);
            pthread_cond_destroy(&_existence);
        }
    };
    template<class T>
    ThreadPool<T>* ThreadPool<T>::_pThreadPool = nullptr;
}