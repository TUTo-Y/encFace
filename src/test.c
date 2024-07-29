#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <locale.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <gtk-3.0/gtk/gtk.h>

#include "config.h"
#include "data.h"
#include "gui_login.h"
#include "gui_play.h"
#include "enc.h"
#include "web.h"
#include "draw.h"

static bool hasreg = false;   // 已注册
static bool haslogin = false; // 已登陆

static SM2_KEY skey;          // 服务器sm2私钥
static SM2_KEY ukey;          // 用户sm2公钥
static char user[0x20] = {0}; // 用户ID

// 人脸特征向量对应的数据 成功0, 失败-1
static int face_msg()
{
}

// 登陆 成功0, 失败-1
static int login()
{
}

// 注册 成功0, 失败-1
static int reg(int client_sockfd)
{
    int ret;
    MSG_TYPE msg;

    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小
    static char ID[0x20] = {0};                        // 用户ID

    DEBUG("|--------------------------注册--------------------------|\n");

    // 接收ID密文长度(size_t)
    ret = recv(client_sockfd, &textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret != -1, "接收ID密文长度失败\n");

    // 接收ID密文(char*)
    ret = recv(client_sockfd, textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret != -1, "接收ID密文失败\n");

    // 解密ID
    sm2_decrypt(&skey, textCipher, textCipherSize, textPlaint, &textPlaintSize);

    // 检测用户是否存在
    textPlaint[textPlaintSize] = '\0';
    strcpy(ID, textPlaint);
    DEBUG("接收到ID: %s\n", textPlaint);
    if(!strcmp(ID, user))
    {
        // 发送用户已存在消息
        DEBUG("用户已存在\n");
        msg = MSG_REGISTER_USER_IN;
        ret = send(client_sockfd, &msg, MSG_TYPE_SIZE, 0);
        goto error;
    }
    msg = MSG_REGISTER_USER_NO;
    ret = send(client_sockfd, &msg, MSG_TYPE_SIZE, 0);
    CHECK(ret != -1, "发送用户不存在消息失败\n");

    // 接收公钥密文长度(size_t)
    ret = recv(client_sockfd, &textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret != -1, "接收公钥密文长度失败\n");

    // 接收公钥密文(char*)
    ret = recv(client_sockfd, textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret != -1, "接收公钥密文失败\n");

    // 解密公钥
    sm2_decrypt(&skey, textCipher, textCipherSize, textPlaint, &textPlaintSize);
    DEBUG("接收到公钥: \n%s\n", textPlaint);

    // 保存用户公钥
    sm2_public_key_info_from_pem_data(&ukey, textPlaint, textPlaintSize);

    // 加密用户ID
    sm2_encrypt(&ukey, ID, strlen(ID), textCipher, &textCipherSize);

    // 发送我的ID的密文长度
    ret = send(client_sockfd, &textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret != -1, "发送ID密文长度失败\n");

    // 发送我的ID的密文
    ret = send(client_sockfd, textCipher, textCipherSize, 0);
    CHECK(ret != -1, "发送ID密文失败\n");

    // 接收ID密文长度(size_t)
    ret = recv(client_sockfd, &textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret != -1, "接收ID密文长度失败\n");

    // 接收ID密文(char*)
    ret = recv(client_sockfd, textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret != -1, "接收ID密文失败\n");

    // 解密ID
    sm2_decrypt(&skey, textCipher, textCipherSize, textPlaint, &textPlaintSize);

    // 对比ID
    textPlaint[textPlaintSize] = '\0';
    if (strcmp(ID, textPlaint))
    {
        // 发送注册失败消息
        DEBUG("两次ID对比不一致\n");
        msg = MSG_ERROR;
        ret = send(client_sockfd, &msg, MSG_TYPE_SIZE, 0);
        goto error;
    }

    // 发送注册成功消息
    msg = MSG_SUCESS;
    ret = send(client_sockfd, &msg, MSG_TYPE_SIZE, 0);
    CHECK(ret != -1, "发送注册成功消息失败\n");

    // 设置注册成功
    strcpy(user, ID);
    hasreg = true;

    DEBUG("|--------------------------end--------------------------|\n");
    return 0;

error:
    perror("注册失败");
    DEBUG("|--------------------------end--------------------------|\n");
    return -1;
}

int test(int argc, char **argv)
{
    int ret;
    MSG_TYPE msg;

    // 读取私钥
    FILE *fp = fopen("sm2_server_private_key.pem", "r");
    sm2_private_key_info_decrypt_from_pem(&skey, "password", fp);
    fclose(fp);

    // 创建socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("创建socket失败");
        return -1;
    }

    // 绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_S);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("绑定端口失败");
        return -1;
    }

    // 监听
    if (listen(sockfd, 5) < 0)
    {
        perror("监听失败");
        return -1;
    }

error:
    while (1)
    {
        haslogin = false;
        hasreg = false;
        memset(user, 0, sizeof(user));

        // 接受连接
        int client_sockfd = accept(sockfd, NULL, 0);
        if (client_sockfd < 0)
        {
            perror("接受连接失败");
            return -1;
        }

        while (1)
        {
            recv(client_sockfd, &msg, MSG_TYPE_SIZE, MSG_WAITALL);
            switch (msg)
            {
            case MSG_CLOSE:
                goto error;
                break;
            case MSG_LOGIN:
                ret = login();
                break;
            case MSG_REGISTER:
                ret = reg(client_sockfd);
                break;
            case MSG_GET_FACE_INFO:
                ret = face_msg();
                break;
            }
        }

        // close
        close(client_sockfd);
        printf("连接已断开\n");
    }

    close(sockfd);
    return 0;
}
