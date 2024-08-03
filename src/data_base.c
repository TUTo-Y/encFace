#include "data_base.h"

void initList(list *l)
{
    l->count = 0;
    l->fd = l;
    l->bk = l;
}

list *createNode()
{
    list *node = (list *)malloc(sizeof(list));

    node->data = NULL;
    node->fd = node;
    node->bk = node;

    return node;
}

list *addDataToNode(list *node, void *data, size_t size, bool isCopy)
{
    if (node == NULL || data == NULL)
        return NULL;

    // 复制数据
    if (isCopy == true)
    {
        node->data = malloc(size);
        memcpy(node->data, data, size);
    }
    // 不复制数据
    else
    {
        node->data = data;
    }

    return node;
}

void addNodeToListStart(list *l, list *node)
{
    if (l == NULL || node == NULL)
        return;

    list *tmp = l->fd;

    l->fd = node;
    node->bk = l;

    tmp->bk = node;
    node->fd = tmp;

    ++l->count;
}

void addNodeToListEnd(list *l, list *node)
{
    if (l == NULL || node == NULL)
        return;

    list *tmp = l->bk;

    l->bk = node;
    node->fd = l;

    tmp->fd = node;
    node->bk = tmp;

    ++l->count;
}

size_t getListCount(list *l)
{
    if (l == NULL)
        return 0;

    return l->count;
}

list *deleteNodeData(list *node, void (*freeData)(void *))
{
    if (node == NULL)
        return NULL;

    // 释放数据
    if (freeData && node->data)
        freeData(node->data);

    node->data = NULL;

    return node;
}

void deleteNode(list *l, list *node, void (*freeData)(void *))
{
    if (node == NULL)
        return;

    // 释放节点数据
    deleteNodeData(node, freeData);

    // 当前为单个节点
    if (node->fd == node && node->bk == node &&
        node != l)
    {
        // 释放节点
        free(node);
        return;
    }

    // 当前节点依然在链表中
    list *fd = node->fd;
    list *bk = node->bk;

    fd->bk = bk;
    bk->fd = fd;

    // 释放节点
    free(node);

    // 减少count
    if (l)
        l->count--;
}

list *getNodeFromListStart(list *l)
{
    if (l == NULL)
        return NULL;

    list *node = l->fd;
    if (node == l)  // 当前链表为空
        return NULL;

    list *fd = node->fd;
    list *bk = node->bk;

    node->fd = node;
    node->bk = node;

    fd->bk = bk;
    bk->fd = fd;

    l->count--;

    return node;
}

list *getNodeFromListEnd(list *l)
{
    if (l == NULL)
        return NULL;

    list *node = l->bk;
    if (node == l)  // 当前链表为空
        return NULL;

    list *fd = node->fd;
    list *bk = node->bk;

    node->fd = node;
    node->bk = node;

    fd->bk = bk;
    bk->fd = fd;

    l->count--;

    return node;
}

void freeList(list *l, void (*freeData)(void *))
{
    while (l->count)
    {
        deleteNode(l, l->fd, freeData);
    }
}

data *Malloc(size_t size)
{
    data *temp = (data *)malloc(sizeof(data));

    if (temp)
    {
        temp->size = size;
        temp->data = malloc(size);
        memset(temp->data, 0, size);
    }
    return temp;
}

void Free(data *data)
{
    if (data)
    {
        if (data->data)
            free(data->data);
        free(data);
    }
}

bool Realloc(data *d, const void *s, size_t size)
{
    if ((!d) || (!s) || size == 0)
        return false;

    void *new_data = realloc(d->data, d->size + size);
    if (new_data == NULL)
    {
        return false;
    }

    // 添加数据
    d->data = new_data;
    memcpy(d->data + d->size, s, size);
    d->size += size;

    return true;
}