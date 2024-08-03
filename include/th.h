#ifndef _TH_H
#define _TH_H

#include <stdbool.h>
#include <pthread.h>

#include "data.h"

// 初始化线程保护
void initThread();

// 检查线程是否存在
bool getThread();

// 设置线程状态
void setThread(bool status);

// 销毁线程保护
void destroyThread();

#endif // _TH_H