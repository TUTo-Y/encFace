#include "vector.h"

/**
 * \brief 从图片中提取特征向量
 * \param image 图片
 * \param head 链表头指针
 * \return 是否成功提取特征向量
 */
bool get_face_vector(const char *image, list **head)
{
    vector vec = {0};
    size_t m = MSG_GET_FACE_VECTOR;
    size_t size = 0;
    int ret = 0;

    // 向facenet服务器发送请求
    ret = send(Global.sock_f, (char *)&m, MSG_TYPE_SIZE, 0);
    CHECK(ret != SOCKET_ERROR, "发送请求失败 : %d\n", WSAGetLastError());

    // 向facenet服务器发送文件名长度
    size = strlen(image);
    ret = send(Global.sock_f, (char*)&size, sizeof(size), 0);
    CHECK(ret != SOCKET_ERROR, "发送文件名长度 : %d\n", WSAGetLastError());

    // 向facenet服务器发送文件名
    ret = send(Global.sock_f, (char*)image, size, 0);
    CHECK(ret != SOCKET_ERROR, "发送文件名失败 : %d\n", WSAGetLastError());

    // facenet返回检测结果
    while (true)
    {
        // 接受请求
        ret = recv(Global.sock_f, (char *)&m, MSG_TYPE_SIZE, 0);
        CHECK(ret != SOCKET_ERROR, "接收请求失败 : %d\n", WSAGetLastError());

        // 检测是否结束
        if (m != MSG_FACE)
            break;

        memset(&vec, 0, sizeof(vec));

        // 人物框位置
        ret = recv(Global.sock_f, (char *)&vec.rect, sizeof(vec.rect), 0);
        CHECK(ret != SOCKET_ERROR, "接收人物框位置失败 : %d\n", WSAGetLastError());
        vec.rect.w -= vec.rect.x;
        vec.rect.h -= vec.rect.y;

        // 返回特征向量大小
        ret = recv(Global.sock_f, (char *)&size, sizeof(size), 0);
        CHECK(ret != SOCKET_ERROR, "接收特征向量大小失败 : %d\n", WSAGetLastError());

        // 创建特征向量空间
        vec.v = (data *)Malloc(size);
        CHECK(vec.v, "内存分配失败\n");

        // 返回特征向量数据
        ret = recv(Global.sock_f, (char *)vec.v->data, size, 0);
        CHECK(ret != SOCKET_ERROR, "接收特征向量数据失败 : %d\n", WSAGetLastError());

        // 添加到链表
        ret = addData(head, &vec, sizeof(vec), true);
        CHECK(ret == true, "添加数据失败\n");
    }

    return true;
error:
    freeList(head, (void (*)(void *))freeVector);
    return false;
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