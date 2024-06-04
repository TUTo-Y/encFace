#ifndef _GUI_LOGIN_H
#define _GUI_LOGIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "enc.h"
#include "main.h"
#include "draw.h"
#include "web.h"
#include "user.h"
#include "gui_text.h"

// 登陆结果
#define LOGIN_ERROR 0
#define LOGIN_SUCCESS 1

/**
 * \brief gui_login界面
 */
unsigned int gui_login();

#endif // _GUI_LOGIN_H