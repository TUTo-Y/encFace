/**
 * 特征向量的提取与处理
 */
#ifndef _VECTORT_H
#define _VECTORT_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <SDL2/SDL.h>

#include "config.h"
#include "data.h"

#define TMP         "tmp"           // 临时文件夹
#define SAVE_FILE   "tmp\\out"      // 保存sig文件
#define BOX_FILE    "tmp\\box"      // 保存box文件
#define VER_FILE    "tmp\\vector"   // 保存vector文件
#define SUCESS      "sucess  "      // 成功标志
#define NOT_FACE    "no face "      // 未检测到人脸
#define NOT_IMAGE   "no image"      // 未检测到图片
#define NOT_FILE    "no file "      // 未检测到文件
#define SIG_SIZE    8               // sig大小

// 人物信息标志
#define NU  0 // flag标志 : 空, 表示未初始化
#define HV  1 // flag标志 : 存在搜索到任务信息
#define NO  2 // flag标志 : 没有搜索到人物信息

#define SEX 0 // sex标志  : 未知
#define MAN 1 // sex标志  : 男性
#define WOM 2 // sex标志  : 女性

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

/**
 * \brief 从图片中提取特征向量
 * \param image 图片
 * \param head 链表头指针
 * \return 是否成功提取特征向量
 */
bool get_face_vector(const char *image, list **head);

/**
 * \brief 释放vec数据
 */
void freeVector(vector *vec);

#endif // _VECTORT_H