/**
 * 基础配置数据
 */
#ifndef _CONFIG_H
#define _CONFIG_H

// debug
#ifdef _DEBUG
#define ERR(...) fprintf(stderr, __VA_ARGS__)
#define DEBUG(...) fprintf(stdout, __VA_ARGS__)
#define DEB(...) __VA_ARGS__
#define REL(...)
#else
#define ERR(...)
#define DEBUG(...)
#define DEB(...)
#define REL(...) __VA_ARGS__
#endif

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

#define VERSION "encFace 1.0" // encFace版本

#define IP_S "192.168.232.131" // 远程服务器地址
#define PORT_S 8080            // 远程服务器端口
#define IP_F "127.0.0.1"       // facenet服务器地址
#define PORT_F 8081            // facenet服务器端口

#define USER_ID "adminTUTo" // 用户ID

#define WINDOW_DEFAULT_WIDTH 1000 // 窗口默认宽度
#define WINDOW_DEFAULT_HEIGHT 800 // 窗口默认高度

#define PYTHON_FACE "face.py" // facenet服务器脚本

#define SM9_PUBLIC "master_public_key.pem" // sm9主公钥文件

#define TTF_PATH "ttf.ttc" // 字体文件

#endif // _CONFIG_H