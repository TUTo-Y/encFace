#include "th.h"

// 初始化线程保护
void initThread()
{
    // 初始化线程锁
    pthread_mutex_init(&Global.lock, NULL);
}

// 检查线程是否存在
bool getThread()
{
    bool ret;
    pthread_mutex_lock(&Global.lock);
    ret = Global.thread_status;
    pthread_mutex_unlock(&Global.lock);
    return ret;
}

// 设置线程状态
void setThread(bool status)
{

    pthread_mutex_lock(&Global.lock);
    Global.thread_status = status;
    pthread_mutex_unlock(&Global.lock);
}

// 销毁线程保护
void destroyThread()
{
    // 销毁线程锁
    pthread_mutex_destroy(&Global.lock);
}
