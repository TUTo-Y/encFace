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

#include <gmssl/sm9.h>
#include <gmssl/sm2.h>
#include <gmssl/zuc.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "config.h"

// 人物信息标志
#define NU 0 // flag标志 : 空, 表示未初始化
#define HV 1 // flag标志 : 存在搜索到任务信息
#define NO 2 // flag标志 : 没有搜索到人物信息

#define SEX 0 // sex标志  : 未知
#define MAN 1 // sex标志  : 男性
#define WOM 2 // sex标志  : 女性

// 全局变量flag标志位
#define FLAG(F) (0x1 << (F))                    // 计算标识位
#define CHECK_FLAG(F) (Global.flag & FLAG(F))   // 检测标识是否为1
#define SET_FLAG(F) (Global.flag |= FLAG(F))    // 将标识标记为1
#define CLEAR_FLAG(F) (Global.flag &= ~FLAG(F)) // 将标识标记为0
enum
{
    flag_start = 0,

    login_button, // 按下登录按钮
    reg_button,   // 按下注册按钮

    text_Click,           // 文本框选中状态
    text_Click_Start,     // 文本框选中状态(初始化状态)
    text_Click_Not,       // 文本框未选中状态
    text_Click_Not_Start, // 文本框未选中状态(初始化状态)

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

typedef unsigned char byte;

// 简单数据结构
typedef struct _data data;
struct _data
{
    byte *data;
    size_t size;
};

// 链表结构
typedef struct _list list;
struct _list
{
    void *data;
    list *next;
};

// 基础信息结构
typedef struct _msg msg;
struct _msg
{
    char name[0x20];     // 姓名
    int age;             // 年龄
    char sex;            // 性别
    char content[0x100]; // 内容
};

// 人物全部信息
typedef struct _vector vector;
struct _vector
{
    int flag;       // 人物是否存在
    data *v;        // BGV加密后的特征向量数据
    SDL_FRect rect; // 位置
    msg info;       // 信息
};

// 全局数据
struct _Global
{
    char path[PATH_MAX]; // 图片路径
    char name[USER_LEN]; // 用户名

    int sock_s; // 远程服务端 socket
    int sock_f; // facenet端 socket

    int quit; // 退出标志

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

/**
 * \brief 向链表中添加一个数据
 * \param head 链表头指针
 * \param data 数据指针
 * \param size 数据大小(如果isCopy为true则必须提供size)
 * \param isCopy 是否复制数据
 */
bool addData(list **head, void *data, size_t size, bool isCopy);

/**
 * \brief 从链表中取出一个数据
 * \param head 链表头指针
 * \param data 数据指针
 * \return 是否成功取出数据
 */
bool getData(list **head, void **data);

/**
 * \brief 释放链表
 * \param head 链表头指针
 * \param Free 释放数据的函数指针
 */
void listFree(list **head, void (*freedom)(void *ptr));

/**
 * \brief 获取链表元素数量
 * \param head 链表头指针
 * \return 链表元素数量
 */
size_t listLen(list *head);

/**
 * \brief 释放vec数据
 */
void freeVector(vector *vec);

/**
 * \brief 创建数据
 * \param size 大小
 */
data *Malloc(size_t size);

/**
 * \brief 释放数据
 * \param data 数据指针
 */
void Free(data *data);

/**
 * \brief 在d后面添加数据
 * \param d 数据
 * \param s 数据
 * \param size s的大小
 */
bool Realloc(data *d, const void *s, size_t size);

#endif