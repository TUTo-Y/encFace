/**
 * 基础数据结构
 */
#ifndef _DATA_BASE_H
#define _DATA_BASE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef unsigned char byte;

// 基础数据结构
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
    union
    {
        void *data;   // 数据(节点)
        size_t count; // 节点数量(链表头)
    };

    list *fd;
    list *bk;
};

/**
 * \brief 初始化链表结构体
 * \param l 链表结构体头指针
 * \return void
 */
void initList(list *l);

/**
 * \brief 创建节点结构体
 * \return 节点指针
 */
list *createNode();

/**
 * \brief 向节点中添加数据
 * \param node 节点指针
 * \param data 数据
 * \param size 数据大小(当且仅当isCopy为true时有效)
 * \param isCopy 是否拷贝数据
 * \return 节点指针
 */
list *addDataToNode(list *node, void *data, size_t size, bool isCopy);

/**
 * \brief 向链表头部添加一个节点
 * \param l 链表头指针
 * \param node 节点指针
 * \return void
 */
void addNodeToListStart(list *l, list *node);

/**
 * \brief 向链表尾部添加一个节点
 * \param l 链表头指针
 * \param node 节点指针
 * \return void
 */
void addNodeToListEnd(list *l, list *node);

/**
 * \brief 获取链表中节点数量
 * \param l 链表结构体头指针
 * \return 节点数量
 */
size_t getListCount(list *l);

/**
 * \brief 删除一个节点的数据
 * \param node 节点指针
 * \param freeData 释放数据函数指针
 * \return 删除数据后的节点指针
 */
list *deleteNodeData(list *node, void (*freeData)(void *));

/**
 * \brief 删除一个节点
 * \param l 链表结构体头指针(可选，如果为NULL则不处理count)
 * \param node 节点指针
 * \param freeData 释放数据函数指针
 * \return void
 */
void deleteNode(list *l, list *node, void (*freeData)(void *));

/**
 * \brief 从链表头部取出一个节点
 * \param l 链表结构体头指针
 * \return 取出的节点指针
 */
list *getNodeFromListStart(list *l);

/**
 * \brief 从链表尾部取出一个节点
 * \param l 链表结构体头指针
 * \return 取出的节点指针
 */
list *getNodeFromListEnd(list *l);

/**
 * \brief 释放链表
 * \param l 链表结构体头指针
 * \param freeData 释放数据函数指针
 * \return void
 */
void freeList(list *l, void (*freeData)(void *));

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

#endif // _DATA_BASE_H