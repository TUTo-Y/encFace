#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define MSG_TYPE_SIZE sizeof(size_t) // 消息类型长度
#define MSG_CLOSE 0                  // 关闭连接消息
#define MSG_SUCESS 1                 // 处理成功
#define MSG_GET_FACE_VECTOR 2        // 请求获取人脸特征向量
#define MSG_GET_FACE_INFO 3          // 请求获取人脸信息

#define MSG_FACE 4     // 人脸数据
#define MSG_FACE_END 5 // 结束

/**
 * 客户端上传特征向量的数据格式:
 *
 * 客户端发送身份ID长度(size_t)
 * 客户端发送身份ID(char*)
 * 客户端发送需要验证的人脸总数量(size_t)
 * 发送第一个人脸数据:
 *      经过sm9加密的ZUC密钥和特征向量的长度(size_t)
 *      经过sm9加密的ZUC密钥和特征向量(char*)
 *      经过ZUC加密后数据的长度(size_t)
 *      经过ZUC加密后数据的数据(char*)
 * 发送第二个人脸数据:
 * ...
 */

/**
 * 服务端返回的数据格式:
 *
 * 第一个角色flag(int)
 * 第一个角色的数据(msg) //不存在则跳过
 * 第二个角色flag(int)
 * 第二个角色的数据(msg) //不存在则跳过
 * ...
 */

// 基础信息结构
typedef struct _msg msg;
struct _msg
{
    char name[0x20];     // 姓名
    int age;             // 年龄
    char sex;            // 性别
    char content[0x100]; // 内容
};

#define PORT 8080

int main()
{
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
        // 接受连接
        int client_sockfd = accept(sockfd, NULL, 0);
        if (client_sockfd < 0)
        {
            perror("接受连接失败");
            return -1;
        }

        while (1)
        {
            size_t m;
            recv(client_sockfd, &m, MSG_TYPE_SIZE, 0);

            // 退出
            if (m == MSG_CLOSE)
                break;

            // 接受数据
            size_t size;
            char *data;

            recv(client_sockfd, &size, sizeof(size_t), 0);
            printf("ID长度: %ld\n", size);

            data = malloc(size + 1);
            recv(client_sockfd, data, size, 0);
            data[size] = '\0';
            printf("身份ID: %s\n", (char *)data);
            free(data);

            recv(client_sockfd, &size, sizeof(size_t), 0);
            printf("人脸总数量: %ld\n", size);

            for (int i = 0; i < size; i++)
            {
                printf("\n正在接受第%d个人脸数据\n", i + 1);
                size_t n;
                recv(client_sockfd, &n, sizeof(size_t), 0);
                printf("sm9加密后长度: %ld\n", n);

                data = malloc(n);
                recv(client_sockfd, data, n, 0);
                free(data);

                recv(client_sockfd, &n, sizeof(size_t), 0);
                printf("ZUC加密后数据的长度: %ld\n", n);

                data = malloc(n);
                recv(client_sockfd, data, n, 0);
                free(data);
            }

            // 发送数据
            srand(time(NULL));
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
                        strcpy(m.name, "张三");
                        break;
                    case 3:
                        strcpy(m.name, "李四");
                        break;
                    case 4:
                        strcpy(m.name, "王五");
                        break;
                    case 5:
                        strcpy(m.name, "老六");
                        break;
                    case 6:
                        strcpy(m.name, "Wald");
                        break;
                    }
                    m.age = rand() % 100;
                    send(client_sockfd, &m, sizeof(msg), 0);

                    printf("发送name = %s\n", m.name);
                }
            }
        }

        // close
        close(client_sockfd);
        printf("连接已断开\n");
    }
    close(sockfd);
    return 0;
}