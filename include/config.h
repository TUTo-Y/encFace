#include <stdio.h>

#ifdef _DEBUG
#define ERR(...) fprintf(stderr, __VA_ARGS__)
#define DEBUG(...) fprintf(stdout, __VA_ARGS__)
#else
#define ERR(...)
#define DEBUG(...)
#endif

#define VERSION "encFace 1.0"

#define IP "192.168.232.131"
#define PORT 8080

#define USER_ID "adminTUTo"

// sm9主公钥目录
#define SM9_PUBLIC "master_public_key.pem"

// 字体目录
#define TTF_PATH "ttf.ttc"

// 获取最大长度，用于处理人物名片
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_OF_THREE(a, b, c) (MAX(MAX(a, b), c))

// 检查错误
#define CHECK(expr, ...)      \
    do                        \
    {                         \
        if (!(expr))          \
        {                     \
            ERR(__VA_ARGS__); \
            goto error;       \
        }                     \
    } while (0)
