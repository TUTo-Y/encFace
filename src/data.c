#include "data.h"

// 全局数据
struct _Global Global = { 0 };

/**
 * \brief 向链表中添加一个数据
 * \param head 链表头指针
 * \param data 数据指针
 * \param size 数据大小(如果isCopy为true则必须提供size)
 * \param isCopy 是否复制数据
 */
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

/**
 * \brief 从链表中取出一个数据
 * \param head 链表头指针
 * \param data 数据指针
 * \return 是否成功取出数据
 */
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

/**
 * \brief 释放链表
 * \param head 链表头指针
 * \param Free 释放数据的函数指针
 */
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

/**
 * \brief 获取链表元素数量
 * \param head 链表头指针
 * \return 链表元素数量
 */
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

/**
 * \brief 释放vec数据
 */
void freeVector(vector *vec)
{
    if (vec)
    {
        if (vec->v)
            Free(vec->v);
        free(vec);
    }
}

/**
 * \brief 创建数据
 * \param size 大小
 */
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

/**
 * \brief 释放数据
 * \param data 数据指针
 */
void Free(data *data)
{
    if (data)
    {
        if (data->data)
            free(data->data);
        free(data);
    }
}

/**
 * \brief 在d后面添加数据
 * \param d 数据
 * \param s 数据
 * \param size s的大小
 */
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