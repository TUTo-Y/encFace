/**
 *      存放所有的数据类型包括全局数据
 *      实现简单的数据结构, 用于快速处理数据
 */
#ifndef _DATA_H
#define _DATA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <sys/types.h>
#include <SDL2/SDL_ttf.h>
#include <GmSSL/sm9.h>

#include "config.h"

// 人物信息标志
#define NU 0 // flag标志 : 空, 表示未初始化
#define HV 1 // flag标志 : 存在搜索到任务信息
#define NO 2 // flag标志 : 没有搜索到人物信息

#define SEX 0 // sex标志  : 未知
#define MAN 1 // sex标志  : 男性
#define WOM 2 // sex标志  : 女性

// 全局变量flag标志位
#define FLAG(F) (0x1 << (F))
#define CHECK_FLAG(F) (Global.flag & FLAG(F))
#define SET_FLAG(F) (Global.flag |= FLAG(F))
#define CLEAR_FLAG(F) (Global.flag &= ~FLAG(F))
enum
{
    image_is_choice = 0,
    image_enter,

    button_enter
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
    void *data; // 不使用data结构是因为还有其他数据类型只需要简单指针，这样可以减少内存占用
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
    // 人物是否存在
    int flag;
    // 加密后的特征向量数据
    data *v;
    // 位置
    SDL_FRect rect;
    // 信息
    msg info;
};

// 全局数据
struct _Global
{
    SOCKET sock_s; // 远程服务端 socket
    SOCKET sock_f; // facenet端 socket

    TTF_Font *font;         // 字体
    SDL_Window *window;     // 窗口
    SDL_Renderer *renderer; // 渲染器

    SDL_Rect windowRect;      // 窗口区域
    SDL_FRect surfaceRect;    // 图片区域
    SDL_FRect buttonRect;     // 按钮区域
    SDL_FRect buttonMsgWRect; // 按钮信息区域
    SDL_FRect buttonMsgHRect; // 按钮信息区域

    SDL_Surface *surface; // 照片Surface
    SDL_Texture *texture; // 照片Texture

    float scale;  // 图片缩放比例(修正宽度比原图像宽度的比例)
    float scale2; // 图片二次缩放比例

    list *face;        // 人脸列表
    list *faceSurface; // 人脸Surface列表
    list *faceTexture; // 人脸Texture列表

    SM9_ENC_MASTER_KEY SM9master;    // sm9主密钥

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
void freeList(list **head, void (*freedom)(void *ptr));

/**
 * \brief 获取链表元素数量
 * \param head 链表头指针
 * \return 链表元素数量
 */
size_t listLength(list *head);

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