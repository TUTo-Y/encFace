/**
 * 基础配置数据
 */
#ifndef _CONFIG_H
#define _CONFIG_H

// 输出字体颜色
#define RESET "\x1B[0m"  // 重置为默认颜色
#define _BLK "\x1B[30m"  // 黑色
#define _RED "\x1B[31m"  // 红色
#define _GRN "\x1B[32m"  // 绿色
#define _YEL "\x1B[33m"  // 黄色
#define _BLU "\x1B[34m"  // 蓝色
#define _MAG "\x1B[35m"  // 洋红色
#define _CYN "\x1B[36m"  // 青色
#define _WHT "\x1B[37m"  // 白色
#define _BBLK "\x1B[90m" // 亮黑色
#define _BRED "\x1B[91m" // 亮红色
#define _BGRN "\x1B[92m" // 亮绿色
#define _BYEL "\x1B[93m" // 亮黄色
#define _BBLU "\x1B[94m" // 亮蓝色
#define _BMAG "\x1B[95m" // 亮洋红色
#define _BCYN "\x1B[96m" // 亮青色
#define _BWHT "\x1B[97m" // 亮白色

#define BLK(str) "\x1B[30m" str RESET  // 黑色
#define RED(str) "\x1B[31m" str RESET  // 红色
#define GRN(str) "\x1B[32m" str RESET  // 绿色
#define YEL(str) "\x1B[33m" str RESET  // 黄色
#define BLU(str) "\x1B[34m" str RESET  // 蓝色
#define MAG(str) "\x1B[35m" str RESET  // 洋红色
#define CYN(str) "\x1B[36m" str RESET  // 青色
#define WHT(str) "\x1B[37m" str RESET  // 白色
#define BBLK(str) "\x1B[90m" str RESET // 亮黑色
#define BRED(str) "\x1B[91m" str RESET // 亮红色
#define BGRN(str) "\x1B[92m" str RESET // 亮绿色
#define BYEL(str) "\x1B[93m" str RESET // 亮黄色
#define BBLU(str) "\x1B[94m" str RESET // 亮蓝色
#define BMAG(str) "\x1B[95m" str RESET // 亮洋红色
#define BCYN(str) "\x1B[96m" str RESET // 亮青色
#define BWHT(str) "\x1B[97m" str RESET // 亮白色

// debug
#ifdef _DEBUG
#define ERR(...) fprintf(stderr, _BRED __VA_ARGS__), fprintf(stderr, RESET);
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

// 设置flag
#define FLAG(F) (0x1 << (F))                    // 计算标识位
#define CHECKFLAG(flag, F) ((flag) & FLAG(F))   // 检查flag是否有F标志
#define SETFLAG(flag, F) ((flag) |= FLAG(F))    // 为flag设置F标志
#define CLEARFLAG(flag, F) ((flag) &= ~FLAG(F)) // 清除flag的F标志

#define VERSION "encFace 2.0" // encFace版本

// #define IP_S "171.115.221.106" // 远程服务器地址
// #define PORT_S 5244            // 远程服务器端口
#define IP_S "127.0.0.1"      // 远程服务器地址
#define PORT_S 8080           // 远程服务器端口
#define IP_F "127.0.0.1"      // facenet服务器地址
#define PORT_F 8081           // facenet服务器端口
#define PYTHON_FACE "face.py" // facenet服务器脚本

#define WINDOW_PLAY_DEFAULT_WIDTH 1300 // play窗口默认宽度
#define WINDOW_PLAY_DEFAULT_HEIGHT 800 // play窗口默认高度

#define WINDOW_LOGIN_DEFAULT_WIDTH 1000 // login窗口默认宽度
#define WINDOW_LOGIN_DEFAULT_HEIGHT 560 // login窗口默认高度

#define USER_LEN 0x20 // 用户ID最长长度

#define FPS 60              // 刷新率
#define FPS_MS (1000 / FPS) // 每一帧的持续时间（毫秒）

#define TO32(size) (((size) + 3) & ~3) // 将不关于4字节对齐的数字转化为关于4字节对齐的数字

#define SM2_PUBLIC "sm2_server_public_key.pem" // sm2服务器公钥文件
#define TTF_PATH "ttf.ttc"                     // 字体文件
#define LOGIN_IMAGE "login.png"                // 登录背景图片
#define USER_CONFIG "user.conf"                // 用户配置文件

#endif // _CONFIG_H