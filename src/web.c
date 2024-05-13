#include "web.h"

/**
 * \brief 建立与远程服务器的连接
 * \return 是否成功
 */
bool connectServer()
{
    struct sockaddr_in server; // 服务器地址
    int ret;

    // 创建一个socket
    Global.sock_s = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(Global.sock_s != INVALID_SOCKET, "创建socket失败 : %d\n", WSAGetLastError());

    // 建立与服务器联系
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_S);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_S); // 服务器地址
    ret = connect(Global.sock_s, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret == 0, "连接服务器失败 : %d\n", WSAGetLastError());

    return true;
error:
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
    system(" start " REL("/B") " python " PYTHON_FACE);
#else
    // 创建子进程
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // 设置进程信息
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // 创建子进程
    ret = CreateProcess(NULL,
                        "python " PYTHON_FACE,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_NO_WINDOW,
                        NULL,
                        NULL,
                        &si,
                        &pi);
    CHECK(ret != 0, "创建facenet服务器失败 : %d\n", GetLastError());
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif

    // 本地启用监听
    SOCKET listSocket = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(listSocket != INVALID_SOCKET, "创建socket失败 : %d\n", WSAGetLastError());

    // 设置本地服务器地址
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_F);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_F); // 服务器地址

    // 绑定socket到服务器地址
    ret = bind(listSocket, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret != SOCKET_ERROR, "绑定socket端口失败 : %d\n", WSAGetLastError());

    // 设置监听时间
    struct timeval timeout;
    timeout.tv_sec = 0;
    ret = setsockopt(listSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != SOCKET_ERROR, "设置监听超时时间失败 : %d\n", WSAGetLastError());

    // 开始监听
    ret = listen(listSocket, 1);
    CHECK(ret != SOCKET_ERROR, "监听socket失败 : %d\n", WSAGetLastError());

    // 接受客户端连接
    Global.sock_f = accept(listSocket, NULL, NULL);
    CHECK(Global.sock_f != INVALID_SOCKET, "接受facenet端连接失败 : %d\n", WSAGetLastError());

    // 设置接收数据时间
    ret = setsockopt(Global.sock_f, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != SOCKET_ERROR, "设置监听超时时间失败 : %d\n", WSAGetLastError());

    closesocket(listSocket);
    return true;
error:
    closesocket(listSocket);
    return false;
}

/**
 * \brief 关闭与远程服务器的连接
 */
void closeServer()
{
    size_t close = MSG_CLOSE;

    // 发送关闭消息
    send(Global.sock_s, (char *)&close, MSG_TYPE_SIZE, 0);

    // 关闭socket
    closesocket(Global.sock_s);
}

/**
 * \brief 关闭与facenet服务器的连接
 */
void closeFaceNet()
{
    size_t close = MSG_CLOSE;

    // 发送关闭消息
    send(Global.sock_f, (char *)&close, MSG_TYPE_SIZE, 0);

    // 关闭socket
    closesocket(Global.sock_f);
}

/**
 * \brief 从图片中提取特征向量
 * \param image 图片
 * \param head 链表头指针
 * \return 是否成功提取特征向量
 */
bool getFaceVector(const wchar_t *image, list **head)
{
    int ret = 0;
    vector vec = {0};
    size_t size = 0;
    MSG_TYPE msgs = MSG_GET_FACE_VECTOR;

    // 向facenet服务器发送请求(size_t)
    ret = send(Global.sock_f, (char *)&msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret != SOCKET_ERROR, "发送请求失败 : %d\n", WSAGetLastError());

    // 向facenet服务器发送文件名长度(size_t)
    size = wcslen(image) * sizeof(wchar_t);
    ret = send(Global.sock_f, (char *)&size, sizeof(size), 0);
    CHECK(ret != SOCKET_ERROR, "发送文件名长度 : %d\n", WSAGetLastError());

    // 向facenet服务器发送文件名(wchar_t*)
    ret = send(Global.sock_f, (char *)image, size, 0);
    CHECK(ret != SOCKET_ERROR, "发送文件名失败 : %d\n", WSAGetLastError());

    // facenet返回检测结果
    while (true)
    {
        // 接受请求(size_t)
        ret = recv(Global.sock_f, (char *)&msgs, MSG_TYPE_SIZE, 0);
        CHECK(ret != SOCKET_ERROR, "接收请求失败 : %d\n", WSAGetLastError());

        // 检测是否结束
        if (MSG_FACE != msgs)
            break;

        memset(&vec, 0, sizeof(vec));

        // 人物框位置(SDL_FRect)
        ret = recv(Global.sock_f, (char *)&vec.rect, sizeof(vec.rect), 0);
        CHECK(ret != SOCKET_ERROR, "接收人物框位置失败 : %d\n", WSAGetLastError());
        vec.rect.w -= vec.rect.x;
        vec.rect.h -= vec.rect.y;

        // 返回特征向量大小(size_t)
        ret = recv(Global.sock_f, (char *)&size, sizeof(size), 0);
        CHECK(ret != SOCKET_ERROR, "接收特征向量大小失败 : %d\n", WSAGetLastError());

        // 创建特征向量空间
        vec.v = (data *)Malloc(size);
        CHECK(vec.v, "内存分配失败\n");

        // 返回特征向量数据(byte*)
        ret = recv(Global.sock_f, (char *)vec.v->data, size, 0);
        CHECK(ret != SOCKET_ERROR, "接收特征向量数据失败 : %d\n", WSAGetLastError());

        // 添加到链表
        ret = addData(head, &vec, sizeof(vec), true);
        CHECK(ret == true, "添加数据失败\n");
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

    // 向远程服务器发送请求(size_t)
    MSG_TYPE msgs = MSG_GET_FACE_INFO;
    ret = send(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret != SOCKET_ERROR, "发送查找申请失败 : %d\n", WSAGetLastError());

    // 发送身份ID长度(size_t)
    size_t id_size = strlen(USER_ID);
    ret = send(Global.sock_s, (char *)&id_size, sizeof(size_t), 0);
    CHECK(ret != SOCKET_ERROR, "发送身份ID长度失败 : %d\n", WSAGetLastError());

    // 发送身份ID(char*)
    ret = send(Global.sock_s, (char *)USER_ID, id_size, 0);
    CHECK(ret != SOCKET_ERROR, "发送身份ID失败 : %d\n", WSAGetLastError());

    // 发送需要验证的人脸总数量(size_t)
    count = listLen(face);
    ret = send(Global.sock_s, (char *)&count, sizeof(size_t), 0);
    CHECK(ret != SOCKET_ERROR, "发送人脸数量失败 : %d\n", WSAGetLastError());

    // 发送数据
    DEB(count = 0);
    list *node = face;
    while (node)
    {
        DEBUG("正在发送第%d个人脸特征向量\n", count + 1);

        char k[ZUC_KEY_SIZE + ZUC_IV_SIZE] = {0}; // ZUC的key和iv
        data *zucOut = NULL;                      // ZUC加密后的数据
        data *sm9Out = NULL;                      // SM9加密后的数据

        // 进行ZUC加密数据
        zucEnc(((vector *)node->data)->v, &zucOut, k, k + ZUC_KEY_SIZE);

        // 进行SM9加密ZUC密钥
        sm9Out = Malloc(SM9_MAX_CIPHERTEXT_SIZE);
        sm9_encrypt(&Global.SM9master, USER_ID, strlen(USER_ID), k, sizeof(k), sm9Out->data, &sm9Out->size);

        // 发送经过sm9加密的ZUC密钥和特征向量的长度(size_t)
        ret = send(Global.sock_s, (char *)&sm9Out->size, sizeof(size_t), 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 经过sm9加密的ZUC密钥长度失败 : %d\n", count + 1, WSAGetLastError());

        // 发送经过sm9加密的ZUC密钥和特征向量(char*)
        ret = send(Global.sock_s, (char *)sm9Out->data, sm9Out->size, 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 经过sm9加密的ZUC密钥失败 : %d\n", count + 1, WSAGetLastError());

        // 发送经过ZUC加密后数据的长度(size_t)
        ret = send(Global.sock_s, (char *)&zucOut->size, sizeof(size_t), 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 发送ZUC加密后数据的长度失败 : %d\n", count + 1, WSAGetLastError());

        // 发送经过ZUC加密后数据的数据(char*)
        ret = send(Global.sock_s, (char *)zucOut->data, zucOut->size, 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 发送ZUC加密后数据失败 : %d\n", count + 1, WSAGetLastError());

        // 释放内存
        Free(zucOut);
        Free(sm9Out);

        node = node->next;
        DEB(count++);
    }

    // 接受服务器回馈
    DEB(count = 0);
    node = face;
    while (node)
    {
        // 接受服务器回馈人物存在标志
        DEBUG("正在接受第%d个人脸数据\n", count + 1);
        ret = recv(Global.sock_s, (char *)&(((vector *)node->data)->flag), sizeof(int), 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 接受服务器回馈flag失败 : %d\n", count + 1, WSAGetLastError());

        // 如果人物存在, 则接受人物信息
        if (((vector *)node->data)->flag == HV)
        {
            ret = recv(Global.sock_s, (char *)&(((vector *)node->data)->info), sizeof(msg), 0);
            CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 接受服务器回馈msg失败 : %d\n", count + 1, WSAGetLastError());
        }

        node = node->next;
        DEB(count++);
    }

    return true;
error:
    return false;
}
