#include "web.h"

/**
 * \brief 通过人脸特征向量获取人脸信息
 * \param face 人脸数据链表
 * \return 是否成功
 */
bool get_face_info(list *face)
{
    WSADATA wsaData;           // winsocket数据
    SOCKET sockfd;             // socket
    struct sockaddr_in server; // 服务器地址
    int ret;
    size_t count = 0;

    // 初始化winsocket
    ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    CHECK(ret == 0, "无法初始化winsocket : %d\n", WSAGetLastError());

    // 创建一个socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    CHECK(sockfd != INVALID_SOCKET, "无法创建socket : %d\n", WSAGetLastError());

    // 建立与服务器联系
    server.sin_family = AF_INET;            // IPv4
    server.sin_port = htons(PORT);          // 服务器端端口
    server.sin_addr.s_addr = inet_addr(IP); // 服务器地址
    ret = connect(sockfd, (struct sockaddr *)&server, sizeof(server));
    CHECK(ret == 0, "连接服务器失败 : %d\n", WSAGetLastError());

    // 读取sm9主公钥
    SM9_ENC_MASTER_KEY master = {0};
    FILE *fp = fopen("master_public_key.pem", "rb");
    CHECK(fp, "无法打开master_public_key.pem\n");
    sm9_enc_master_public_key_from_pem(&master, fp);
    fclose(fp);

    // 发送身份ID长度(size_t)
    size_t id_size = strlen(USER_ID);
    ret = send(sockfd, (char *)&id_size, sizeof(size_t), 0);
    CHECK(ret != SOCKET_ERROR, "发送身份ID长度失败 : %d\n", WSAGetLastError());
    DEBUG("发送身份ID长度 : %d 字节\n", id_size);

    // 发送身份ID(char*)
    ret = send(sockfd, (char *)USER_ID, id_size, 0);
    CHECK(ret != SOCKET_ERROR, "发送身份ID失败 : %d\n", WSAGetLastError());

    // 发送需要验证的人脸总数量(size_t)
    count = listLength(face);
    ret = send(sockfd, (char *)&count, sizeof(size_t), 0);
    CHECK(ret != SOCKET_ERROR, "发送人脸数量失败 : %d\n", WSAGetLastError());
    DEBUG("发送人脸数量 : %d 个\n", count);

// 发送数据
#ifdef _DEBUG
    count = 0;
#endif
    list *node = face;
    while (node)
    {
        DEBUG("正在发送第%d个人脸数据\n", count + 1);

        char k[ZUC_KEY_SIZE + ZUC_IV_SIZE] = {0}; // ZUC的key和iv
        data *zucOut = NULL;                      // ZUC加密后的数据
        data *sm9Out = NULL;                      // SM9加密后的数据

        // 进行ZUC加密数据
        zucEnc(((vector *)node->data)->v, &zucOut, k, k + ZUC_KEY_SIZE);

        // 进行SM9加密ZUC密钥
        sm9Out = Malloc(SM9_MAX_CIPHERTEXT_SIZE);
        sm9_encrypt(&master, USER_ID, strlen(USER_ID), k, sizeof(k), sm9Out->data, &sm9Out->size);

        // 发送经过sm9加密的ZUC密钥和特征向量的长度(size_t)
        ret = send(sockfd, (char *)&sm9Out->size, sizeof(size_t), 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 经过sm9加密的ZUC密钥长度失败 : %d\n", count + 1, WSAGetLastError());
        DEBUG("发送经过sm9加密的ZUC密钥长度 : %d 字节\n", sm9Out->size);

        // 发送经过sm9加密的ZUC密钥和特征向量(char*)
        ret = send(sockfd, (char *)sm9Out->data, sm9Out->size, 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 经过sm9加密的ZUC密钥失败 : %d\n", count + 1, WSAGetLastError());

        // 发送经过ZUC加密后数据的长度(size_t)
        ret = send(sockfd, (char *)&zucOut->size, sizeof(size_t), 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 发送ZUC加密后数据的长度失败 : %d\n", count + 1, WSAGetLastError());
        DEBUG("发送ZUC加密后数据的长度 : %d 字节\n", zucOut->size);

        // 发送经过ZUC加密后数据的数据(char*)
        ret = send(sockfd, (char *)zucOut->data, zucOut->size, 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 发送ZUC加密后数据失败 : %d\n", count + 1, WSAGetLastError());

        // 释放内存
        Free(zucOut);
        Free(sm9Out);

        node = node->next;
#ifdef _DEBUG
        count++;
#endif
    }

    // 接受服务器回馈
#ifdef _DEBUG
    count = 0;
#endif
    node = face;
    while (node)
    {
        // 接受服务器回馈人物存在标志
        DEBUG("正在接受第%d个人脸数据\n", count + 1);
        ret = recv(sockfd, (char *)&(((vector *)node->data)->flag), sizeof(int), 0);
        CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 接受服务器回馈flag失败 : %d\n", count + 1, WSAGetLastError());

        // 如果人物存在, 则接受人物信息
        if (((vector *)node->data)->flag == HV)
        {
            ret = recv(sockfd, (char *)&(((vector *)node->data)->info), sizeof(msg), 0);
            CHECK(ret != SOCKET_ERROR, "在处理%d个人脸数据时, 接受服务器回馈msg失败 : %d\n", count + 1, WSAGetLastError());

            DEBUG("姓名: %s\n", ((vector *)node->data)->info.name);
            DEBUG("年龄: %d\n", ((vector *)node->data)->info.age);
            DEBUG("性别id: %d\n\n", ((vector *)node->data)->info.sex);
        }

#ifdef _DEBUG
        count++;
#endif
        node = node->next;
    }

    // 关闭socket
    closesocket(sockfd);
    WSACleanup();
    return true;

error:
    if (sockfd != INVALID_SOCKET)
        closesocket(sockfd);
    WSACleanup();
    return false;
}