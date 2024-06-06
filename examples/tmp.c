#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <gmssl/zuc.h>
#include <gmssl/sm2.h>

// 简单数据结构
typedef unsigned char byte;
typedef struct _data data;
struct _data
{
    byte *data;
    size_t size;
};

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
#define CHECK(...)
#define TO32(size) (((size) + 3) & ~3)
#define MSG_TYPE size_t
#define MSG_TYPE_SIZE sizeof(MSG_TYPE) // 消息类型长度
#define MSG_CLOSE 0                    // 关闭连接消息
#define MSG_SUCESS 1                   // 处理成功

#define MSG_GET_FACE_VECTOR 2 // 请求获取人脸特征向量
#define MSG_GET_FACE_INFO 3   // 请求获取人脸信息

#define MSG_FACE 4     // 人脸数据
#define MSG_FACE_END 5 // 结束

#define MSG_REGISTER 6         // 注册请求
#define MSG_REGISTER_USER_IN 7 // 注册用户存在

#define MSG_LOGIN 8 // 登录请求

// 基础信息结构
typedef struct _msg msg;
struct _msg
{
    char name[0x20];     // 姓名
    int age;             // 年龄
    char sex;            // 性别
    char content[0x100]; // 内容
};
bool userGet(const char *filename, const char *ID, SM2_KEY *sm2_key, uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE]);

#define PORT 8080
#define FILENAME "../user.config"
#define NAME "123"

int main()
{
    SM2_KEY sm2_key = {0};
    ZUC_STATE zuc = {0};
    uint8_t key[ZUC_KEY_SIZE] = {0};
    uint8_t iv[ZUC_KEY_SIZE] = {0};

    // 读取用户信息
    userGet(FILENAME, NAME, &sm2_key, key, iv);

    // 计算出ZUC密钥
    zuc_init(&zuc, key, iv);

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
    addr.sin_port = htons(PORT);
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

    while (1)
    {
        int ret;
        // 接受连接
        int client_sockfd = accept(sockfd, NULL, 0);
        if (client_sockfd < 0)
        {
            perror("接受连接失败");
            return -1;
        }

        while (1)
        {
            MSG_TYPE m;
            recv(client_sockfd, &m, MSG_TYPE_SIZE, MSG_WAITALL);

            // 退出
            if (m == MSG_CLOSE)
                break;

            // 接受数据
            size_t size;
            char *content;

            // 接受人脸总数
            recv(client_sockfd, &size, sizeof(size_t), MSG_WAITALL);
            printf("人脸总数量: %ld\n", size);

            for (int i = 0; i < size; i++)
            {
                printf("\n正在接受第%d个人脸数据\n", i + 1);
                size_t n;

                // 接受长度
                recv(client_sockfd, &n, sizeof(size_t), MSG_WAITALL);
                printf("sm9加密后长度: %ld\n", n);

                // 接受数据
                content = malloc(n);
                recv(client_sockfd, content, n, MSG_WAITALL);
                free(content);
            }

            // 发送数据
            srand(time(NULL));

            // 生成ZUC密钥流
            size_t k_n = TO32(sizeof(msg)) / sizeof(ZUC_UINT32);
            ZUC_UINT32 *k = malloc(k_n * sizeof(ZUC_UINT32));
            zuc_generate_keystream(&zuc, k_n, k);

            for (int i = 0; i < size; i++)
            {
                int flag;
                flag = rand() % 2 + 1;
                send(client_sockfd, &flag, sizeof(int), 0);
                if (flag == 1)
                {
                    msg m = {0};
                    switch (rand() % 7)
                    {
                    case 0:
                        strcpy(m.name, "老大");
                        break;
                    case 1:
                        strcpy(m.name, "老二");
                        break;
                    case 2:
                        strcpy(m.name, "老三");
                        break;
                    case 3:
                        strcpy(m.name, "老四");
                        break;
                    case 4:
                        strcpy(m.name, "老五");
                        break;
                    case 5:
                        strcpy(m.name, "老六");
                        break;
                    case 6:
                        strcpy(m.name, "老七");
                        break;
                    }
                    m.age = rand() % 30 + 10;

                    {

                        // 加密人物信息
                        data *info = Malloc(sizeof(msg));
                        CHECK(info != NULL);
                        for (int i = 0; i < k_n; i++)
                            ((ZUC_UINT32 *)info->data)[i] = ((ZUC_UINT32 *)(&m))[i] ^ k[i];

                        // 上传人物信息
                        ret = send(client_sockfd, info->data, sizeof(msg), 0);
                        CHECK(ret);
                        Free(info);
                    }

                    printf("发送name = %s\n", m.name);
                }
            }
            free(k);
        }

        // close
        close(client_sockfd);
        printf("连接已断开\n");
    }
    close(sockfd);
    return 0;
}

/**
 * \brief 从用户文件中获取用户信息
 * \param filename 用户文件名
 * \param ID 用户名
 * \param sm2_key sm2用户密钥对
 * \param key ZUC密钥
 * \param iv ZUC初始化向量
 * \return
 */
bool userGet(const char *filename, const char *ID, SM2_KEY *sm2_key, uint8_t key[ZUC_KEY_SIZE], uint8_t iv[ZUC_KEY_SIZE])
{
    // 检查参数
    if (!filename || !ID || !sm2_key || !key || !iv)
        return false;

    // 打开用户文件
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
        return false;

    size_t len = 0;       // 用户名长度
    char name[512] = {0}; // 用户名

    uint8_t public_key_info[512] = {0};                 // sm2公钥信息
    const uint8_t *public_key_info_p = public_key_info; // 保存sm2公钥信息
    size_t public_key_info_len = 0;                     //  sm2公钥信息长度

    uint8_t private_key_info[512] = {0};                  // sm2私钥信息
    const uint8_t *private_key_info_p = private_key_info; // 保存sm2私钥信息
    size_t private_key_info_len = 0;                      // sm2私钥信息长度

    // 循环读取所有用户信息
    while (!feof(file))
    {
        // 读取用户名
        fread(&len, sizeof(size_t), 1, file);
        fread(name, sizeof(char), len, file);

        // 读取sm2公钥
        fread(&public_key_info_len, sizeof(size_t), 1, file);
        fread(public_key_info, sizeof(char), public_key_info_len, file);

        // 读取sm2私钥
        fread(&private_key_info_len, sizeof(size_t), 1, file);
        fread(private_key_info, sizeof(char), private_key_info_len, file);

        // 读取ZUC密钥
        fread(key, sizeof(uint8_t), ZUC_KEY_SIZE, file);

        // 读取ZUC初始化向量
        fread(iv, sizeof(uint8_t), ZUC_KEY_SIZE, file);

        // 对比用户名
        if (!strcmp(name, ID))
        {
            // 读取成功, 保存sm2公钥
            sm2_public_key_info_from_der(sm2_key, &public_key_info_p, &public_key_info_len);

            // 读取成功, 保存sm2私钥
            sm2_private_key_from_der(sm2_key, &private_key_info_p, &private_key_info_len);

            // 关闭文件
            fclose(file);
            return true;
        }
    }

    fclose(file);
    return false;
}