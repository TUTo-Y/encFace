/**
 * 基础配置数据
*/
#ifndef _CONFIG_H
#define _CONFIG_H

// debug
#ifdef _DEBUG
#define ERR(...) fprintf(stderr, __VA_ARGS__)
#define DEBUG(...) fprintf(stdout, __VA_ARGS__)
#else
#define ERR(...)
#define DEBUG(...)
#endif

#define VERSION "encFace 1.0"   // encFace版本

#define IP "192.168.232.131"    // 服务器地址
#define PORT 8080              // 服务器端口

#define USER_ID "adminTUTo"     // 用户ID

#define SM9_PUBLIC "master_public_key.pem" // sm9主公钥文件

#define TTF_PATH "ttf.ttc"      // 字体文件

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

#endif  // _CONFIG_H