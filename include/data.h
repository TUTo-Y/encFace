/**
 * 简单的数据结构, 用于快速处理数据
 * 包含data结构，链表结构
 */
#ifndef _DATA_H
#define _DATA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"

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
    size_t size;
    list *next;
};

/**
 * \brief 向链表中添加一个数据
 * \param head 链表头指针
 * \param data 数据指针
 * \param size 数据大小
 * \param isCopy 是否复制数据
 * \return true添加成功
 */
bool addData(list **head, void *data, size_t size, bool isCopy);

/**
 * \brief 从链表中取出一个数据
 * \param head 链表头指针
 * \param data 数据指针
 * \param size 数据大小(可省略)
 * \return 是否成功取出数据
 * \warning 请注意数据的释放
 */
bool getData(list **head, void **data, size_t *size);

/**
 * \brief 释放链表
 * \param head 链表头指针
 * \param Free 释放数据的函数指针
 */
void freeList(list **head, void (*freedom)(void *data));

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