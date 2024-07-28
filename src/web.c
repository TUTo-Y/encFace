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
    CHECK(Global.sock_s != -1, "创建socket错误 : %s\n", strerror(errno));

    // 建立与服务器联系
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_S);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_S); // 服务器地址
    ret = connect(Global.sock_s, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret != -1, "连接远程服务器错误: %s\n", strerror(errno));

    // 设置超时时间
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    ret = setsockopt(Global.sock_s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != -1, "设置 socket 接收超时失败: %s\n", strerror(errno));

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
    CHECK(listSocket != -1, "创建socket错误: %s\n", strerror(errno));

    // 设置socket属性
    int optval = 1;
    setsockopt(listSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    CHECK(ret != -1, "设置socket属性错误: %s\n", strerror(errno));

    // 设置本地服务器地址
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_F);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_F); // 服务器地址

    // 绑定socket到服务器地址
    ret = bind(listSocket, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret != -1, "绑定socket端口错误: %s\n", strerror(errno));

    // 设置监听时间
    struct timeval timeout = {0};
    timeout.tv_sec = 5;
    ret = setsockopt(listSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != -1, "设置监听超时时间错误: %s\n", strerror(errno));

    // 开始监听
    ret = listen(listSocket, 0);
    CHECK(ret != -1, "监听socket错误: %s\n", strerror(errno));

    // 接受客户端连接
    Global.sock_f = accept(listSocket, NULL, NULL);
    CHECK(Global.sock_f != -1, "接受facenet端连接错误: %s\n", strerror(errno));

    // 设置接收数据时间
    ret = setsockopt(Global.sock_f, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != -1, "设置超时时间错误: %s\n", strerror(errno));

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
    MSG_TYPE msgs = MSG_CLOSE;

    // 发送关闭消息
    if (send(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, 0) == INADDR_NONE)
        return;

    // 关闭socket
    close(Global.sock_s);
}

/**
 * \brief 关闭与facenet服务器的连接
 */
void closeFaceNet()
{
    MSG_TYPE msgs = MSG_CLOSE;

    // 发送关闭消息
    if (send(Global.sock_f, (char *)&msgs, MSG_TYPE_SIZE, 0) == INADDR_NONE)
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

        // 发送BGV加密后的长度
        ret = send(Global.sock_s, (char *)&(((vector *)node->data)->v->size), sizeof(size_t), 0);
        CHECK(ret != INADDR_NONE, "在向远程服务器发送第%d个人脸特征向量时, 发送BGV加密后的长度错误: %s\n", count, strerror(errno));

        // 发送BGV加密后的数据
        ret = send(Global.sock_s, (char *)(((vector *)node->data)->v->data), ((vector *)node->data)->v->size, 0);
        CHECK(ret != INADDR_NONE, "在向远程服务器发送第%d个人脸特征向量时, 发送BGV加密后的数据错误: %s\n", count, strerror(errno));

        node = node->next;
    }

    // 生成ZUC密钥流
    size_t k_n = TO32(sizeof(msg)) / sizeof(ZUC_UINT32);
    ZUC_UINT32 *k = malloc(k_n * sizeof(ZUC_UINT32));
    zuc_generate_keystream(&Global.ZUCstate, k_n, k);

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
            // 加密数据
            ret = recv(Global.sock_s, (char *)&(((vector *)node->data)->info), sizeof(msg), MSG_WAITALL);
            CHECK(ret != INADDR_NONE, "在从远程服务器接受第%d个人脸数据时, 接受服务器回馈msg错误: %s\n", count, strerror(errno));

            // 对数据进行解密
            for (int i = 0; i < k_n; i++)
                ((ZUC_UINT32 *)&(((vector *)node->data)->info))[i] ^= k[i];
        }

        node = node->next;
    }

    // 释放资源
    free(k);
    return true;
error:
    // 释放资源
    free(k);
    return false;
}

/**
 * \brief 注册
 */
bool registerUser(const char *ID, const SM2_KEY *skey, SM2_KEY *mkey)
{
    int ret;
    MSG_TYPE msgs;

    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小

    data *pub_pem = NULL;

    // 检查参数
    if (!ID || !skey || !mkey)
        return false;

    // 向服务器发送注册请求
    msgs = MSG_REGISTER;
    ret = send(Global.sock_s, &msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret == MSG_TYPE_SIZE, "向远程服务器发送注册请求错误: %s\n", strerror(errno));

    // 加密ID
    ret = sm2_encrypt(skey, ID, strlen(ID), textCipher, &textCipherSize);
    CHECK(ret == 1, "加密ID错误\n");

    // 发送ID密文长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret == sizeof(textCipherSize), "向远程服务器发送ID密文长度错误: %s\n", strerror(errno));

    // 发送ID密文(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, 0);
    CHECK(ret == textCipherSize, "向远程服务器发送ID密文错误: %s\n", strerror(errno));

    // 检测用户是否存在
    // ret = recv(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
    // CHECK(ret == MSG_TYPE_SIZE, "从远程服务器接受用户是否存在错误: %s\n", strerror(errno));
    // CHECK(msgs == MSG_REGISTER_USER_NO, "用户已存在\n");

    // 生成客户端sm2密钥对
    ret = sm2_key_generate(mkey);
    CHECK(ret == 1, "生成sm2密钥对错误\n");

    // 获取sm2公钥的pem数据
    sm2_public_key_info_to_pem_data(mkey, &pub_pem);

    // 加密公钥
    ret = sm2_encrypt(skey, pub_pem->data, pub_pem->size, textCipher, &textCipherSize);
    CHECK(ret == 1, "加密公钥错误\n");

    printf("公钥长度: %ld\n", textCipherSize);

    // 发送公钥密文长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret == sizeof(textCipherSize), "向远程服务器发送公钥密文长度错误: %s\n", strerror(errno));

    // 发送公钥密文(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, 0);
    CHECK(ret == textCipherSize, "向远程服务器发送公钥密文错误: %s\n", strerror(errno));

    // 接受我的ID的密文长度
    ret = recv(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret == sizeof(textCipherSize), "从远程服务器接受我的ID的密文长度错误: %s\n", strerror(errno));

    // 接受我的ID的密文
    ret = recv(Global.sock_s, (char *)textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret == textCipherSize, "从远程服务器接受我的ID的密文错误: %s\n", strerror(errno));

    // 解密我的ID
    ret = sm2_decrypt(mkey, textCipher, textCipherSize, textPlaint, &textPlaintSize);
    CHECK(ret == 1, "解密我的ID错误\n");

    // 校验我的ID
    textPlaint[textPlaintSize] = '\0';
    CHECK(strcmp(ID, textPlaint) == 0, "接受到得ID是错误的:[%s]\n", textPlaint);

    // 加密ID
    ret = sm2_encrypt(skey, ID, strlen(ID), textCipher, &textCipherSize);
    CHECK(ret == 1, "加密ID错误\n");

    // 发送ID密文长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret == sizeof(textCipherSize), "向远程服务器发送ID密文长度错误: %s\n", strerror(errno));

    // 发送ID密文(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret == textCipherSize, "向远程服务器发送ID密文错误: %s\n", strerror(errno));

    // 接受消息
    ret = recv(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
    CHECK(ret == MSG_TYPE_SIZE, "从远程服务器接受消息错误: %s\n", strerror(errno));

    // 检查是否注册成功
    CHECK(msgs == MSG_SUCESS, "注册失败\n");

    Free(pub_pem);
    return true;

error:
    Free(pub_pem);
    return false;
}

/**
 * \brief 登录
 */
bool loginUser(const char *ID)
{
    int ret;

    return true;
error:
    return false;
}