/**
 * 基础配置数据
 */
#ifndef _CONFIG_H
#define _CONFIG_H

// debug
#ifdef _DEBUG
#define ERR(...) fprintf(stderr, "\x1B[91m" __VA_ARGS__), fprintf(stderr, "\x1B[0m");
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

#define VERSION "encFace 2.0" // encFace版本

// #define IP_S "154.12.18.126"  // 远程服务器地址
#define IP_S "127.0.0.1" // 远程服务器地址
#define IP_F "127.0.0.1" // facenet服务器地址
// #define PORT_S 5244           // 远程服务器端口
#define PORT_S 8080           // 远程服务器端口
#define PORT_F 8081           // facenet服务器端口
#define PYTHON_FACE "face.py" // facenet服务器脚本

#define WINDOW_PLAY_DEFAULT_WIDTH 1000 // play窗口默认宽度
#define WINDOW_PLAY_DEFAULT_HEIGHT 800 // play窗口默认高度
// 1.7802
#define WINDOW_LOGIN_DEFAULT_WIDTH 1000 // login窗口默认宽度
#define WINDOW_LOGIN_DEFAULT_HEIGHT 560 // login窗口默认高度

#define USER_ID "adminTUTo" // 用户ID
#define USER_LEN 0x20       // 用户ID长度

#define SM9_PUBLIC "master_public_key.pem"     // sm9主公钥文件
#define SM2_PUBLIC "sm2_server_public_key.pem" // sm2服务器公钥文件

#define FPS 60              // 刷新率
#define FPS_MS (1000 / FPS) // 每一帧的持续时间（毫秒）

#define TO32(size) (((size) + 3) & ~3) // 将不关于4字节对齐的数字转化为关于4字节对齐的数字

#define TTF_PATH "ttf.ttc" // 字体文件

// #define TEAM_IMAGE "team.jpg"        // 团队图片
#define TEAM_IMAGE NULL              // 团队图片
#define LOGIN_IMAGE "login.png"      // 登录图片
#define LOGIN_BACK_IMAGE "login.jpg" // 登录背景图片
#define USER_CONFIG "user.config"    // 用户配置文件

#endif // _CONFIG_H