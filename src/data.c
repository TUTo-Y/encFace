#include "data.h"

/**
 * \brief 向链表中添加一个数据
 * \param head 链表头指针
 * \param data 数据指针
 * \param size 数据大小
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
 * \param size 数据大小(可省略)
 * \return 是否成功取出数据
 */
bool getData(list **head, void **data, size_t *size)
{
    if (data)
        *data = NULL;
    if (size)
        *size = 0;
    if ((!head) || (*head == NULL))
        return false;

    list *temp = *head;
    *head = (*head)->next;

    if (data)
        *data = temp->data;
    if (size)
        *size = temp->size;

    free(temp);
    return true;
}

/**
 * \brief 释放链表
 * \param head 链表头指针
 */
void freeList(list **head, void (*freedom)(void *data))
{
    if (!head)
        return;

    list *temp = NULL;
    while (*head)
    {
        temp = *head;
        *head = (*head)->next;
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
size_t listLength(list *head)
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
 * \brief 创建数据
 * \param size 大小
 */
data *Malloc(size_t size)
{
    data *temp = (data *)malloc(sizeof(data));
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

    d->data = new_data;
    memcpy(d->data + d->size, s, size);
    d->size += size;

    return true;
}