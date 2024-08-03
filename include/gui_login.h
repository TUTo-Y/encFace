#ifndef _GUI_LOGIN_H
#define _GUI_LOGIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "th.h"
#include "enc.h"
#include "web.h"
#include "user.h"
#include "main.h"
#include "draw.h"
#include "gui_msg.h"
#include "gui_text.h"
#include "gui_button.h"

// 登陆结果
#define LOGIN_SUCCESS 0
#define LOGIN_ERROR 1

// 登陆函数传参
typedef struct _loginThreadParam
{
    char name[USER_LEN]; // 用户名
    int *quit;           // 退出标志
    int *ret;            // 返回值
    guiMsg *msg;         // 消息
} loginThreadParam;

// 注册函数传参
typedef struct _regThreadParam
{
    char name[USER_LEN]; // 用户名
    guiMsg *msg;         // 消息
} regThreadParam;

/**
 * \brief gui_login界面
 */
unsigned int gui_login();

#endif // _GUI_LOGIN_H