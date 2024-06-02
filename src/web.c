#include "web.h"

/**
 * \brief 建立与远程服务器的连接
 * \return 是否成功
 */
bool connectServer()
{
    struct sockaddr_in server; // 服务器地址
    int ret;

    // 创建客户端socket
    Global.sock_s = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(Global.sock_s != INADDR_NONE, "创建socket错误 : %s\n", strerror(errno));

    // 建立与服务器联系
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_S);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_S); // 服务器地址
    ret = connect(Global.sock_s, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret != INADDR_NONE, "连接远程服务器错误: %s\n", strerror(errno));

    return true;
error:
    close(Global.sock_s);
    return false;
}

/**
 * \brief 建立与facenet服务器的连接
 * \return 是否成功
 */
bool connectFaceNet()
{
    struct sockaddr_in server; // 服务器地址
    int ret;

    // 在本地创建服务器程序
#ifdef _DEBUG
    system(" python " PYTHON_FACE "&");
#else
    system(" python " PYTHON_FACE "&");
#endif

    // 创建监听socket
    int listSocket = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(listSocket != INADDR_NONE, "创建socket错误: %s\n", strerror(errno));

    // 设置socket属性
    int optval = 1;
    setsockopt(listSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    CHECK(ret != INADDR_NONE, "设置socket属性错误: %s\n", strerror(errno));

    // 设置本地服务器地址
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_F);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_F); // 服务器地址

    // 绑定socket到服务器地址
    ret = bind(listSocket, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret != INADDR_NONE, "绑定socket端口错误: %s\n", strerror(errno));

    // 设置监听时间
    struct timeval timeout = { 0 };
    timeout.tv_sec = 10;
    ret = setsockopt(listSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != INADDR_NONE, "设置监听超时时间错误: %s\n", strerror(errno));

    // 开始监听
    ret = listen(listSocket, 0);
    CHECK(ret != INADDR_NONE, "监听socket错误: %s\n", strerror(errno));

    // 接受客户端连接
    Global.sock_f = accept(listSocket, NULL, NULL);
    CHECK(Global.sock_f != INADDR_NONE, "接受facenet端连接错误: %s\n", strerror(errno));

    // 设置接收数据时间
    ret = setsockopt(Global.sock_f, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != INADDR_NONE, "设置超时时间错误: %s\n", strerror(errno));

    close(listSocket);
    return true;
error:
    close(listSocket);
    close(Global.sock_f);
    return false;
}

/**
 * \brief 关闭与远程服务器的连接
 */
void closeServer()
{
    size_t msgs = MSG_CLOSE;
    
    // 发送关闭消息
    if(send(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, 0) == INADDR_NONE)
        return;

    // 关闭socket
    close(Global.sock_s);
}

/**
 * \brief 关闭与facenet服务器的连接
 */
void closeFaceNet()
{
    size_t msgs = MSG_CLOSE;

    // 发送关闭消息
    if(send(Global.sock_f, (char *)&msgs, MSG_TYPE_SIZE, 0) == INADDR_NONE)
        return;

    // 关闭socket
    close(Global.sock_f);
}

/**
 * \brief 从图片中提取特征向量
 * \param image 图片
 * \param head 链表头指针
 * \return 是否成功提取特征向量
 */
bool getFaceVector(const char *file, list **head)
{
    int ret = 0;
    vector vec = {0};
    size_t size = 0;
    MSG_TYPE msgs = 0;

    // 检查参数
    CHECK(file && head, "文件名为空\n");
    
    // 初始化
    listFree(head, (void (*)(void *))freeVector);

    // 向facenet服务器发送请求(size_t)
    msgs = MSG_GET_FACE_VECTOR;
    ret = send(Global.sock_f, (char *)&msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret != INADDR_NONE, "向faceNet服务器发送请求错误: %s\n", strerror(errno));

    // 向facenet服务器发送文件名长度(size_t)
    size = strlen(file);
    ret = send(Global.sock_f, (char *)&size, sizeof(size), 0);
    CHECK(ret != INADDR_NONE, "向faceNet服务器发送文件名长度错误: %s\n", strerror(errno));

    // 向facenet服务器发送文件名(wchar_t*)
    ret = send(Global.sock_f, (char *)file, size, 0);
    CHECK(ret != INADDR_NONE, "向faceNet服务器发送文件名错误: %s\n", strerror(errno));

    // facenet返回检测结果
    DEB(size_t count = 0);
    while (true)
    {
        // 接受请求(size_t)
        ret = recv(Global.sock_f, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "从faceNet服务器接收faceNet服务器请求错误: %s\n", strerror(errno));

        // 检测是否结束
        if (MSG_FACE != msgs)
            break;

        DEBUG("正在从faceNet服务器获取第%d个人脸特征向量\n", ++count);

        memset(&vec, 0, sizeof(vec));

        // 人物框位置(SDL_FRect)
        ret = recv(Global.sock_f, (char *)&vec.rect, sizeof(vec.rect), MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "从faceNet服务器接收人物框位置错误: %s\n", strerror(errno));
        vec.rect.w -= vec.rect.x;
        vec.rect.h -= vec.rect.y;

        // 返回特征向量大小(size_t)
        ret = recv(Global.sock_f, (char *)&size, sizeof(size), MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "从faceNet服务器接收特征向量大小错误: %s\n", strerror(errno));

        // 创建特征向量空间
        vec.v = (data *)Malloc(size);
        CHECK(vec.v, "特征向量空间内存分配错误\n");

        // 返回特征向量数据(byte*)
        ret = recv(Global.sock_f, (char *)vec.v->data, size, MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "从faceNet服务器接收特征向量数据错误: %s\n", strerror(errno));

        // 添加到链表
        ret = addData(head, &vec, sizeof(vec), true);
        CHECK(ret == true, "添加数据错误\n");
    }

    return true;
error:
    listFree(head, (void (*)(void *))freeVector);
    return false;
}

/**
 * \brief 通过人脸特征向量获取人脸信息
 * \param face 人脸数据链表
 * \return 是否成功
 */
bool getFaceInfo(list *face)
{
    int ret;
    size_t count = 0;
    MSG_TYPE msgs = 0;

    // 向远程服务器发送请求(size_t)
    msgs = MSG_GET_FACE_INFO;
    ret = send(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送查找请求错误: %s\n", strerror(errno));

    // 发送身份ID长度(size_t)
    size_t id_size = strlen(USER_ID);
    ret = send(Global.sock_s, (char *)&id_size, sizeof(size_t), 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送身份ID长度错误: %s\n", strerror(errno));

    // 发送身份ID(char*)
    ret = send(Global.sock_s, (char *)USER_ID, id_size, 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送身份ID错误: %s\n", strerror(errno));

    // 发送需要验证的人脸总数量(size_t)
    count = listLen(face);
    ret = send(Global.sock_s, (char *)&count, sizeof(size_t), 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送人脸数量错误: %s\n", strerror(errno));

    // 发送数据
    DEB(count = 0);
    list *node = face;
    while (node)
    {
        DEBUG("正在向远程服务器发送第%d个人脸特征向量\n", ++count);

        char k[ZUC_KEY_SIZE + ZUC_IV_SIZE] = {0}; // ZUC的key和iv
        data *zucOut = NULL;                      // ZUC加密后的数据
        data *sm9Out = NULL;                      // SM9加密后的数据

        // 进行ZUC加密数据
        zucEnc(((vector *)node->data)->v, &zucOut, k, k + ZUC_KEY_SIZE);

        // 进行SM9加密ZUC密钥
        sm9Out = Malloc(SM9_MAX_CIPHERTEXT_SIZE);
        sm9_encrypt(&Global.SM9master, USER_ID, strlen(USER_ID), k, sizeof(k), sm9Out->data, &sm9Out->size);

        // 发送经过sm9加密的ZUC密钥的长度(size_t)
        ret = send(Global.sock_s, (char *)&sm9Out->size, sizeof(size_t), 0);
        CHECK(ret != INADDR_NONE, "在向远程服务器发送第%d个人脸特征向量时, 发送经过sm9加密的ZUC密钥长度错误: %s\n", count, strerror(errno));

        // 发送经过sm9加密的ZUC密钥(char*)
        ret = send(Global.sock_s, (char *)sm9Out->data, sm9Out->size, 0);
        CHECK(ret != INADDR_NONE, "在向远程服务器发送第%d个人脸特征向量时, 发送经过sm9加密的ZUC密钥错误: %s\n", count, strerror(errno));

        // 发送经过ZUC加密后数据的长度(size_t)
        ret = send(Global.sock_s, (char *)&zucOut->size, sizeof(size_t), 0);
        CHECK(ret != INADDR_NONE, "在向远程服务器发送第%d个人脸特征向量时, 发送ZUC加密后的数据的长度错误: %s\n", count, strerror(errno));

        // 发送经过ZUC加密后数据的数据(char*)
        ret = send(Global.sock_s, (char *)zucOut->data, zucOut->size, 0);
        CHECK(ret != INADDR_NONE, "在向远程服务器发送第%d个人脸特征向量时, 发送ZUC加密后的数据错误: %s\n", count, strerror(errno));

        // 释放内存
        Free(zucOut);
        Free(sm9Out);

        node = node->next;
    }

    // 接受服务器回馈
    DEB(count = 0);
    node = face;
    while (node)
    {
        // 接受服务器回馈人物存在标志
        DEBUG("正在接受第%d个人脸数据\n", ++count);
        ret = recv(Global.sock_s, (char *)&(((vector *)node->data)->flag), sizeof(int), MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "在从远程服务器接受第%d个人脸数据时, 接受服务器回馈flag错误: %s\n", count, strerror(errno));

        // 如果人物存在, 则接受人物信息
        if (((vector *)node->data)->flag == HV)
        {
            ret = recv(Global.sock_s, (char *)&(((vector *)node->data)->info), sizeof(msg), MSG_WAITALL);
            CHECK(ret != INADDR_NONE, "在从远程服务器接受第%d个人脸数据时, 接受服务器回馈msg错误: %s\n", count, strerror(errno));
        }

        node = node->next;
    }

    return true;
error:
    return false;
}
