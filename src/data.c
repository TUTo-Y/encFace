#include "data.h"

// 全局数据
struct _Global Global = { 0 };

bool addData(list **head, void *data, size_t size, bool isCopy)
{
    if (!head || !data)
        return false;

    // 创建新的节点
    list *newNode = (list *)malloc(sizeof(list));
    if (newNode == NULL)
        return false;

    // 复制数据
    if (isCopy == true)
    {
        newNode->data = malloc(size);
        if (newNode->data == NULL)
        {
            free(newNode);
            return false;
        }
        memcpy(newNode->data, data, size);
    }
    else
    {
        newNode->data = data;
    }

    // 添加新节点
    newNode->next = *head;
    *head = newNode;
    return true;
}

bool getData(list **head, void **data)
{
    if ((!head) || (*head == NULL))
        return false;

    // 取出节点
    list *temp = *head;
    *head = (*head)->next;

    // 取出数据
    if (data)
        *data = temp->data;
    else
        *data = NULL;

    // 释放节点
    free(temp);
    return true;
}

void listFree(list **head, void (*freedom)(void *ptr))
{
    if (!head)
        return;

    // 释放链表
    list *temp = NULL;
    while (*head)
    {
        temp = *head;
        *head = (*head)->next;
        // 释放数据
        if (temp->data && freedom)
            freedom(temp->data);
        free(temp);
    }
}

size_t listLen(list *head)
{
    size_t count = 0;
    while (head)
    {
        count++;
        head = head->next;
    }
    return count;
}

void freeVector(vector *vec)
{
    if (vec)
    {
        if (vec->v)
            Free(vec->v);
        free(vec);
    }
}

data *Malloc(size_t size)
{
    data *temp = (data *)malloc(sizeof(data));
    if (temp == NULL)
        return NULL;

    if (temp)
    {
        temp->size = size;
        temp->data = malloc(size);
        if (temp->data == NULL)
        {
            free(temp);
            return NULL;
        }
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