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

// 全局变量flag标志位
#define CHECK_FLAG(F) (Global.flag & FLAG(F))   // 检测标识是否为1
#define SET_FLAG(F) (Global.flag |= FLAG(F))    // 将标识标记为1
#define CLEAR_FLAG(F) (Global.flag &= ~FLAG(F)) // 将标识标记为0
enum
{
    image_enter,  // 图片是否按下
    button_enter, // 按钮是否按下

    button_In, // 是否在按钮内

    button_Select,           // 按钮选中动画
    button_Select_Start,     // 按钮选中动画(初始化状态)
    button_Not_Select,       // 按钮离开动画
    button_Not_Select_Start, // 按钮离开动画(初始化状态)

    button_Click,           // 按钮点击动画
    button_Click_Start,     // 按钮点击动画(初始化状态)
    button_Not_Click,       // 按钮松开动画
    button_Not_Click_Start, // 按钮松开动画(初始化状态)

    button_Wait,           // 按钮等待动画
    button_Wait_Start,     // 按钮等待动画(初始化状态)
    button_Not_Wait_Start, // 按钮等待结束动画(初始化状态)
};

// 基础信息结构
typedef struct _basicMsg basicMsg;
struct _basicMsg
{
    char name[0x20];    // 姓名
    char id[0x20];      // 学号
    char college[0x20]; // 学院
    char major[0x20];   // 专业
    char class[0x20];   // 班级
    char phone[0x20];   // 电话
    char email[0x20];   // 邮箱
    char address[0x20]; // 地址
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

    SM2_KEY SM2server;  // sm2服务器公钥
    SM2_KEY SM2user;    // sm2用户密钥对
    ZUC_STATE ZUCstate; // ZUC密钥

    pthread_t thread;     // 线程
    bool thread_status;   // 线程状态, 线程是否存在
    pthread_mutex_t lock; // 互斥锁

    size_t flag; // 标志
};
extern struct _Global Global;

void freePersonal(personal *p);

#endif