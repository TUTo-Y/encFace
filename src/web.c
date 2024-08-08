#include "web.h"

/**
 * \brief 建立与远程服务器的连接
 * \return 是否成功
 */
bool connectServer()
{
    struct sockaddr_in server = {0}; // 服务器地址
    int ret;

    // 创建客户端socket
    Global.sock_s = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(Global.sock_s != INADDR_NONE);

    // 建立与服务器联系
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_S);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_S); // 服务器地址
    ret = connect(Global.sock_s, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret != INADDR_NONE);

    // 设置超时时间
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    ret = setsockopt(Global.sock_s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != INADDR_NONE);

    return true;

error:
    ERR("无法连接到远程服务器：\n", strerror(errno));
    if (Global.sock_s)
        close(Global.sock_s);
    return false;
}

/**
 * \brief 建立与facenet服务器的连接
 * \return 是否成功
 */
bool connectFaceNet()
{
    struct sockaddr_in server = {0}; // 服务器地址
    int listSocket = 0;
    int ret;

    // 在本地创建服务器程序
#ifdef _DEBUG
    system(" python3 " PYTHON_FACE "&");
#else
    system(" python3 " PYTHON_FACE "&");
#endif

    // 创建监听socket
    listSocket = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(listSocket != INADDR_NONE);

    // 设置socket属性
    int optval = 1;
    setsockopt(listSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    CHECK(ret != INADDR_NONE);

    // 设置本地服务器地址
    server.sin_family = AF_INET;              // IPv4
    server.sin_port = htons(PORT_F);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP_F); // 服务器地址

    // 绑定socket到服务器地址
    ret = bind(listSocket, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret != INADDR_NONE);

    // 设置监听时间
    struct timeval timeout = {0};
    timeout.tv_sec = 5;
    ret = setsockopt(listSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != INADDR_NONE);

    // 开始监听
    ret = listen(listSocket, 0);
    CHECK(ret != INADDR_NONE);

    // 接收客户端连接
    Global.sock_f = accept(listSocket, NULL, NULL);
    CHECK(Global.sock_f != INADDR_NONE);

    // 设置接收数据时间
    ret = setsockopt(Global.sock_f, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    CHECK(ret != INADDR_NONE);

    if (Global.sock_f)
        close(listSocket);
    return true;

error:
    ERR("无法连接到facenet服务器：\n", strerror(errno));
    if (listSocket)
        close(listSocket);
    if (Global.sock_f)
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
 * \brief 从facenet服务器获取人脸特征向量
 * \param file 图片
 * \param l 人物列表
 * \return 是否成功提取特征向量
 */
bool getFaceVector(const char *file, list *l)
{
    int ret = 0;
    size_t size = 0;
    personal p = {0};

    if (file == NULL || l == NULL)
        goto error;

    // 向facenet服务器发送请求(size_t)
    MSG_TYPE msgs = MSG_GET_FACE_VECTOR;
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
        // 接收请求(size_t)
        ret = recv(Global.sock_f, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "从faceNet服务器接收faceNet服务器请求错误: %s\n", strerror(errno));

        // 检测是否结束
        if (MSG_FACE != msgs)
            break;

        DEBUG("正在从faceNet服务器获取第%d个人脸特征向量\n", ++count);

        memset(&p, 0, sizeof(p));

        // 人物框位置(SDL_FRect)
        ret = recv(Global.sock_f, (char *)&p.rect, sizeof(p.rect), MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "从faceNet服务器接收人物框位置错误: %s\n", strerror(errno));
        p.rect.w -= p.rect.x;
        p.rect.h -= p.rect.y;

        // 返回特征向量大小(size_t)
        ret = recv(Global.sock_f, (char *)&size, sizeof(size), MSG_WAITALL);
        CHECK(ret != INADDR_NONE, "从faceNet服务器接收特征向量大小错误: %s\n", strerror(errno));

        // 创建特征向量空间
        p.vector = (data *)Malloc(size);

        DEBUG("BGV加密后大小为：%d\n", p.vector->size);

        // 返回特征向量数据(byte*)
        recv(Global.sock_f, (char *)p.vector->data, size, MSG_WAITALL);

        // 添加到链表
        addNodeToListEnd(l, addDataToNode(createNode(), &p, sizeof(p), true));
    }
    return true;

error:
    return false;
}

/**
 * \brief 从远程服务器获取人物信息
 * \param p 人物信息
 * \return 是否成功
 */
bool getFaceInfo(personal *p)
{
    int ret;
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_IV_SIZE] = {0};
    ZUC_STATE state = {0};

    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小

    // check
    if (p == NULL || p->vector == NULL)
        goto error;

    // 向服务器发送请求识别
    MSG_TYPE msgs = MSG_GET_FACE_INFO;
    ret = send(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送查找请求错误: %s\n", strerror(errno));

    // 发送特征向量大小(size_t)
    ret = send(Global.sock_s, (char *)&p->vector->size, sizeof(size_t), 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送特征向量大小错误: %s\n", strerror(errno));

    // 发送特征向量(char*)
    ret = send(Global.sock_s, (char *)p->vector->data, p->vector->size, 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送特征向量错误: %s\n", strerror(errno));

    // 接收服务器回馈(是否检测到人物存在)
    ret = recv(Global.sock_s, (char *)&p->flag, sizeof(p->flag), MSG_WAITALL);
    CHECK(ret != INADDR_NONE, "从远程服务器接收人物存在标识错误: %s\n", strerror(errno));

    // 如果人物不存在, 则返回
    if (p->flag == NU)
        return true;

    // 接受被sm2加密后的ZUC密钥长度(size_t)
    ret = recv(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret != INADDR_NONE, "从远程服务器接收ZUC密钥长度错误: %s\n", strerror(errno));

    // 接受被sm2加密后的ZUC密钥(char*)
    ret = recv(Global.sock_s, (char *)textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret != INADDR_NONE, "从远程服务器接收ZUC密钥错误: %s\n", strerror(errno));

    // 解密ZUC密钥
    ret = sm2_decrypt(&Global.SM2user, textCipher, textCipherSize, textPlaint, &textPlaintSize);
    CHECK(ret == 1, "解密ZUC密钥错误\n");
    memcpy(key, textPlaint, ZUC_KEY_SIZE);
    memcpy(iv, textPlaint + ZUC_KEY_SIZE, ZUC_IV_SIZE);

    // 接受服务端传送来的人物数据(char*)
    ret = recv(Global.sock_s, (char *)&p->info, sizeof(p->info), MSG_WAITALL);
    CHECK(ret != INADDR_NONE, "从远程服务器接收人物数据错误: %s\n", strerror(errno));

    // 解密人物数据
    zuc_init(&state, key, iv);
    zuc_encrypt(&state, (byte *)&p->info, sizeof(p->info), (byte *)&p->info);
    zuc_init(&Global.ZUCstate, Global.ZUC_key, Global.ZUC_iv);
    zuc_encrypt(&Global.ZUCstate, (byte *)&p->info, sizeof(p->info), (byte *)&p->info);

    DEBUG("接受到人物姓名: %s\n", p->info.name);

    return true;
error:
    return false;
}

/**
 * \brief 上传人物数据
 * \param p 人物信息
 * \return 是否成功
 */
bool uploadFaceInfo(const personal *p)
{
    int ret;
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_IV_SIZE] = {0};
    ZUC_STATE state;

    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小

    if (p == NULL)
        goto error;

    // 发送请求
    MSG_TYPE msgs = MSG_UPLOAD_FACE_INFO;
    ret = send(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送上传请求错误: %s\n", strerror(errno));

    // 发送特征向量大小(size_t)
    ret = send(Global.sock_s, (char *)&p->vector->size, sizeof(size_t), 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送特征向量大小错误: %s\n", strerror(errno));

    // 发送特征向量(char*)
    ret = send(Global.sock_s, (char *)p->vector->data, p->vector->size, 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送特征向量错误: %s\n", strerror(errno));

    // 生成ZUC密钥
    zucKeyIv(key, iv);

    // sm2加密ZUC密钥
    memcpy(textPlaint, key, ZUC_KEY_SIZE);
    memcpy(textPlaint + ZUC_KEY_SIZE, iv, ZUC_IV_SIZE);
    textPlaintSize = ZUC_KEY_SIZE + ZUC_IV_SIZE;
    ret = sm2_encrypt(&Global.SM2server, textPlaint, textPlaintSize, textCipher, &textCipherSize);
    CHECK(ret == 1, "加密ZUC密钥错误\n");

    // 发送ZUC密钥长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送ZUC密钥长度错误: %s\n", strerror(errno));

    // 发送ZUC密钥(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送ZUC密钥错误: %s\n", strerror(errno));

    // 使用ZUC密钥加密人物数据
    basicMsg tmp = {0};
    memcpy(&tmp, &p->info, sizeof(p->info));
    zuc_init(&state, key, iv);
    zuc_encrypt(&state, (byte *)&tmp, sizeof(tmp), (byte *)&tmp);
    zuc_init(&Global.ZUCstate, Global.ZUC_key, Global.ZUC_iv);
    zuc_encrypt(&Global.ZUCstate, (byte *)&tmp, sizeof(tmp), (byte *)&tmp);

    // 发送人物数据(char*)
    ret = send(Global.sock_s, (char *)&tmp, sizeof(tmp), 0);
    CHECK(ret != INADDR_NONE, "向远程服务器发送人物数据错误: %s\n", strerror(errno));

    // 接收结果
    ret = recv(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
    CHECK(ret != INADDR_NONE, "从远程服务器接收消息错误: %s\n", strerror(errno));

    CHECK(msgs == MSG_SUCESS, "上传人物数据失败\n");

    return true;
error:
    return false;
}

/**
 * \brief 注册
 */
int registerUser(const char *ID, const SM2_KEY *skey, SM2_KEY *ukey)
{
    int ret;
    MSG_TYPE msgs;

    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小

    data *pub_pem = NULL;

    // 检查参数
    if (!ID || !skey || !ukey)
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
    ret = recv(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
    CHECK(ret == MSG_TYPE_SIZE, "从远程服务器接收用户存在标识错误: %s\n", strerror(errno));
    CHECK(msgs == MSG_REGISTER_USER_NO, "用户已存在\n");

    // 生成客户端sm2密钥对
    ret = sm2_key_generate(ukey);
    CHECK(ret == 1, "生成sm2密钥对错误\n");

    // 获取sm2公钥的pem数据
    sm2_public_key_info_to_pem_data(ukey, &pub_pem);

    // 加密公钥
    ret = sm2_encrypt(skey, pub_pem->data, pub_pem->size, textCipher, &textCipherSize);
    CHECK(ret == 1, "加密公钥错误\n");

    // 发送公钥密文长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret == sizeof(textCipherSize), "向远程服务器发送公钥密文长度错误: %s\n", strerror(errno));

    // 发送公钥密文(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, 0);
    CHECK(ret == textCipherSize, "向远程服务器发送公钥密文错误: %s\n", strerror(errno));

    // 接收我的ID的密文长度
    ret = recv(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret == sizeof(textCipherSize), "从远程服务器接收我的ID的密文长度错误: %s\n", strerror(errno));

    // 接收我的ID的密文
    ret = recv(Global.sock_s, (char *)textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret == textCipherSize, "从远程服务器接收我的ID的密文错误: %s\n", strerror(errno));

    // 解密我的ID
    ret = sm2_decrypt(ukey, textCipher, textCipherSize, textPlaint, &textPlaintSize);
    CHECK(ret == 1, "解密我的ID错误\n");

    // 校验我的ID
    textPlaint[textPlaintSize] = '\0';
    CHECK(strcmp(ID, textPlaint) == 0, "接收到得ID是错误的:[%s]\n", textPlaint);

    // 加密ID
    ret = sm2_encrypt(skey, ID, strlen(ID), textCipher, &textCipherSize);
    CHECK(ret == 1, "加密ID错误\n");

    // 发送ID密文长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret == sizeof(textCipherSize), "向远程服务器发送ID密文长度错误: %s\n", strerror(errno));

    // 发送ID密文(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret == textCipherSize, "向远程服务器发送ID密文错误: %s\n", strerror(errno));

    // 接收消息
    ret = recv(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
    CHECK(ret == MSG_TYPE_SIZE, "从远程服务器接收消息错误: %s\n", strerror(errno));

    // 检查是否注册成功
    CHECK(msgs == MSG_SUCESS, "注册失败\n");

    Free(pub_pem);
    return 0;

error:
    Free(pub_pem);

    // 用户存在错误
    if (MSG_REGISTER_USER_IN == msgs)
        return 2;
    // 其他错误
    return 1;
}

/**
 * \brief 登录
 */
int loginUser(const char *ID, const SM2_KEY *skey, const SM2_KEY *ukey)
{
    int ret;
    MSG_TYPE msgs;

    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小

    // 接收发送登陆请求
    msgs = MSG_LOGIN;
    ret = send(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, 0);
    CHECK(ret == MSG_TYPE_SIZE, "向远程服务器发送登陆请求错误: %s\n", strerror(errno));

    // 加密ID
    ret = sm2_encrypt(skey, ID, strlen(ID), textCipher, &textCipherSize);
    CHECK(ret == 1, "加密ID错误\n");

    // 发送ID密文长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret == sizeof(textCipherSize), "向远程服务器发送ID密文长度错误: %s\n", strerror(errno));

    // 发送ID密文(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, 0);
    CHECK(ret == textCipherSize, "向远程服务器发送ID密文错误: %s\n", strerror(errno));

    // 接收是否存在
    ret = recv(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
    CHECK(ret == MSG_TYPE_SIZE, "从远程服务器接收用户存在标识错误: %s\n", strerror(errno));
    CHECK(msgs == MSG_LOGIN_USER_IN, "用户不存在\n");

    // 接收签名长度(size_t)
    ret = recv(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret == sizeof(textCipherSize), "从远程服务器接收签名长度错误: %s\n", strerror(errno));

    // 接收签名(char*)
    ret = recv(Global.sock_s, (char *)textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret == textCipherSize, "从远程服务器接收签名错误: %s\n", strerror(errno));

    // 解密签名
    ret = sm2_decrypt(ukey, textCipher, textCipherSize, textPlaint, &textPlaintSize);
    CHECK(ret == 1, "解密签名错误\n");

    // 加密签名
    ret = sm2_encrypt(skey, textPlaint, textPlaintSize, textCipher, &textCipherSize);
    CHECK(ret == 1, "加密签名错误\n");

    // 发送签名长度(size_t)
    ret = send(Global.sock_s, (char *)&textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret == sizeof(textCipherSize), "向远程服务器发送签名长度错误: %s\n", strerror(errno));

    // 发送签名(char*)
    ret = send(Global.sock_s, (char *)textCipher, textCipherSize, 0);
    CHECK(ret == textCipherSize, "向远程服务器发送签名错误: %s\n", strerror(errno));

    // 接收结果
    ret = recv(Global.sock_s, (char *)&msgs, MSG_TYPE_SIZE, MSG_WAITALL);
    CHECK(ret == MSG_TYPE_SIZE, "从远程服务器接收消息错误: %s\n", strerror(errno));

    // 检查是否登陆成功
    CHECK(msgs == MSG_SUCESS, "登陆失败\n");

    return 0;
error:
    // 用户不存在错误
    if (MSG_LOGIN_USER_NO == msgs)
        return 2;
    return 1;
}