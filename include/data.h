/**
 *      存放所有的数据类型包括全局数据
 *      实现简单的数据结构, 用于快速处理数据
 */
#ifndef _DATA_H
#define _DATA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/limits.h>

#include <gmssl/sm2.h>
#include <gmssl/zuc.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include "config.h"
#include "data_base.h"

// 人物信息标志
#define NU 0 // flag标志 : 空, 表示没有搜索到人物信息
#define HV 1 // flag标志 : 存在搜索到任务信息

// 基础信息结构
typedef struct _basicMsg basicMsg;
struct _basicMsg
{
    char name[0x20];    // 姓名
    char id[0x20];      // 学号
    char college[0x20]; // 学院
    char major[0x20];   // 专业班级
};

// 人物全部信息
typedef struct _personal personal;
struct _personal
{
    size_t flag; // 人物是否存在

    data *vector;   // BGV加密后的特征向量数据
    SDL_FRect rect; // 位置

    basicMsg info;            // 信息
    SDL_FRect infoRect;       // 信息位置
    SDL_Texture *infoTexture; // 信息纹理
};

// 全局数据
struct _Global
{
    int sock_s; // 远程服务端 socket
    int sock_f; // facenet端 socket

    TTF_Font *font; // 字体

    SM2_KEY SM2server;             // sm2服务器公钥
    SM2_KEY SM2user;               // sm2用户密钥对
    ZUC_STATE ZUCstate;            // ZUC密钥
    uint8_t ZUC_key[ZUC_KEY_SIZE]; // ZUC密钥
    uint8_t ZUC_iv[ZUC_IV_SIZE];   // ZUC向量

    pthread_t thread;     // 线程
    bool thread_status;   // 线程状态, 线程是否存在
    pthread_mutex_t lock; // 互斥锁

    size_t flag; // 标志
};
extern struct _Global Global;

void freePersonal(personal *p);
#endif