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
static ZUC_STATE zstate;      // zuc状态
static char user[0x20] = {0}; // 用户ID

// 人脸特征向量对应的数据 成功0, 失败-1
static int face_msg(int client_sockfd)
{
    int ret;
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_IV_SIZE] = {0};
    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小

    if (haslogin == false)
    {
        // 发送未登录消息
        DEBUG("未登录\n");
        return -1;
    }

    // 接收人脸特征向量长度
    size_t size;
    ret = recv(client_sockfd, &size, sizeof(size), MSG_WAITALL);
    printf("接受到的数据长度为：%d\n", size);

    char *data = malloc(size);
    ret = recv(client_sockfd, data, size, MSG_WAITALL);
    free(data);


    size_t flag = rand() % 2;
    // 发送人物是否存在标识
    ret = send(client_sockfd, &flag, sizeof(size_t), 0);
    CHECK(ret != -1, "发送人物是否存在标识失败\n");

    if (flag == HV)
    {
        // 生成ZUC密钥
        ZUC_STATE state;
        uint8_t key[16];
        uint8_t iv[16];

        zucKeyVi(key, iv);
        zuc_init(&state, key, iv);

        // 加密ZUC密钥
        memcpy(textPlaint, key, 16);
        memcpy(textPlaint + 16, iv, 16);
        textPlaintSize = 32;
        sm2_encrypt(&ukey, textPlaint, textPlaintSize, textCipher, &textCipherSize);

        // 发送ZUC密钥长度
        ret = send(client_sockfd, &textCipherSize, sizeof(textCipherSize), 0);
        CHECK(ret != -1, "发送ZUC密钥长度失败\n");

        // 发送ZUC密钥
        ret = send(client_sockfd, textCipher, textCipherSize, 0);
        CHECK(ret != -1, "发送ZUC密钥失败\n");
        
        basicMsg info = {0};
        switch (rand() % 10)
        {
        case 0:
            strcpy(info.name, "张三");
            break;
        case 1:
            strcpy(info.name, "李四");
            break;
        case 2:
            strcpy(info.name, "王五");
            break;
        case 3:
            strcpy(info.name, "赵六");
            break;
        case 4:
            strcpy(info.name, "孙七");
            break;
        case 5:
            strcpy(info.name, "周八");
            break;
        case 6:
            strcpy(info.name, "吴九");
            break;
        case 7:
            strcpy(info.name, "郑十");
            break;
        case 8:
            strcpy(info.name, "钱十一");
            break;
        case 9:
            strcpy(info.name, "马十二");
            break;
        }

        // 加密信息
        zucEnc((byte *)&info, (byte *)&info, sizeof(info), &zstate);
        zucEnc((byte *)&info, (byte *)&info, sizeof(info), &state);

        // 发送信息
        ret = send(client_sockfd, &info, sizeof(info), 0);
        CHECK(ret != -1, "发送信息失败\n");
    }

    return 0;
error:
    return -1;
}

// 登陆 成功0, 失败-1
static int login(int client_sockfd)
{
    int ret;
    MSG_TYPE msg;

    uint8_t textCipher[SM2_MAX_CIPHERTEXT_SIZE] = {0}; // sm2密文
    size_t textCipherSize = 0;                         // sm2密文大小
    uint8_t textPlaint[SM2_MAX_PLAINTEXT_SIZE] = {0};  // sm2明文
    size_t textPlaintSize = 0;                         // sm2明文大小
    static char ID[0x20] = {0};                        // 用户ID

    // 接收ID密文长度(size_t)
    ret = recv(client_sockfd, &textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret != -1, "接收ID密文长度失败\n");

    // 接收ID密文(char*)
    ret = recv(client_sockfd, textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret != -1, "接收ID密文失败\n");

    // 解密ID
    sm2_decrypt(&skey, textCipher, textCipherSize, textPlaint, &textPlaintSize);

    // 检查用户是否存在
    textPlaint[textPlaintSize] = '\0';
    strcpy(ID, textPlaint);
    if (strcmp(ID, user) && hasreg == false)
    {
        // 发送用户不存在消息
        DEBUG("用户不存在\n");
        msg = MSG_LOGIN_USER_NO;
        ret = send(client_sockfd, &msg, MSG_TYPE_SIZE, 0);
        goto error;
    }

    // 发送存在/不存在消息
    msg = MSG_LOGIN_USER_IN;
    ret = send(client_sockfd, &msg, MSG_TYPE_SIZE, 0);
    CHECK(ret != -1, "发送用户存在消息失败\n");

    // 读取数据库中的信息
    uint8_t key[16];
    uint8_t iv[16];
    memset(&ukey, 0, sizeof(ukey));
    userGet(USER_CONFIG, ID, &ukey, key, iv);
    zuc_init(&zstate, key, iv);

    strcpy(textPlaint, "tuto666");
    textPlaintSize = strlen(textPlaint);
    // 加密签名
    ret = sm2_encrypt(&ukey, textPlaint, textPlaintSize, textCipher, &textCipherSize);

    // 发送签名长度
    ret = send(client_sockfd, &textCipherSize, sizeof(textCipherSize), 0);
    CHECK(ret != -1, "发送签名长度失败\n");

    // 发送签名
    ret = send(client_sockfd, textCipher, textCipherSize, 0);
    CHECK(ret != -1, "发送签名失败\n");

    // 接收签名长度
    ret = recv(client_sockfd, &textCipherSize, sizeof(textCipherSize), MSG_WAITALL);
    CHECK(ret != -1, "接收签名长度失败\n");

    // 接收签名
    ret = recv(client_sockfd, textCipher, textCipherSize, MSG_WAITALL);
    CHECK(ret != -1, "接收签名失败\n");

    // 解密签名
    sm2_decrypt(&skey, textCipher, textCipherSize, textPlaint, &textPlaintSize);

    // 发送登陆成功消息
    msg = MSG_SUCESS;
    send(client_sockfd, &msg, MSG_TYPE_SIZE, 0);

    // 设置登陆成功
    haslogin = true;

    return 0;

error:
    perror("登陆失败");
    return -1;
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
    DEBUG("注册接收到ID: %s\n", textPlaint);
    if (!strcmp(ID, user))
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

    return 0;

error:
    perror("注册失败");
    return -1;
}

int test(int argc, char **argv)
{
    int ret;
    MSG_TYPE msg;

    srand(time(NULL));
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
                ret = login(client_sockfd);
                break;
            case MSG_REGISTER:
                ret = reg(client_sockfd);
                break;
            case MSG_GET_FACE_INFO:
                ret = face_msg(client_sockfd);
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
